// main.cpp
// Transcoded from user's Python codec_final.py
// Uses OpenCV for image IO and Eigen for matrix operations.
// Build with CMake (provided).

#include <iostream>
#include <cmath>
#include <string>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>

using namespace std;
using Eigen::MatrixXd;

// Quantization matrix Q (standard-like)
static const double Q_data[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109,103,77},
    {24, 35, 55, 64, 81, 104,113,92},
    {49, 64, 78, 87, 103,121,120,101},
    {72, 92, 95, 98, 112,100,103,99}
};

// build matrices P and Q as Eigen
MatrixXd buildP() {
    MatrixXd P(8,8);
    for (int k=0; k<8; ++k) {
        for (int i=0; i<8; ++i) {
            double Ck = (k==0) ? std::sqrt(1.0/2.0) : 1.0;
            P(k,i) = std::sqrt(2.0/8.0) * Ck * std::cos((M_PI/(2.0*8.0)) * (2.0*i + 1.0) * k);
        }
    }
    return P;
}

MatrixXd buildQ() {
    MatrixXd Q(8,8);
    for (int i=0;i<8;++i) for (int j=0;j<8;++j) Q(i,j) = Q_data[i][j];
    return Q;
}

// Helper: truncate image size to multiple of 8
cv::Mat truncateToMultipleOf8(const cv::Mat& img) {
    int x = img.rows / 8 * 8;
    int y = img.cols / 8 * 8;
    return img(cv::Range(0, x), cv::Range(0, y)).clone();
}

// bruit: zero out high-frequency coefficients with mask (i+j > n)
MatrixXd applyBruit(const MatrixXd& block, int n) {
    MatrixXd M = block;
    for (int i=0;i<8;++i) {
        for (int j=0;j<8;++j) {
            if ((i + j) > n) M(i,j) = 0.0;
        }
    }
    return M;
}

// compress single-channel (8-bit) image into DCT quantized blocks
// returns compressed matrix (blocks replaced by their masked DCT coefficients) and count nonzero
pair<cv::Mat, long long> compressChannel(const cv::Mat& channel, const MatrixXd& P, const MatrixXd& Q, int n=8) {
    cv::Mat tr = truncateToMultipleOf8(channel);
    int rows = tr.rows, cols = tr.cols;
    cv::Mat out = cv::Mat::zeros(rows, cols, CV_64F); // store D (quantized) in double
    long long nonzeroCount = 0;

    // Process block by block
    for (int bi=0; bi<rows; bi += 8) {
        for (int bj=0; bj<cols; bj += 8) {
            // copy 8x8 into Eigen matrix, subtract 128
            MatrixXd A(8,8);
            for (int i=0;i<8;++i) for (int j=0;j<8;++j)
                A(i,j) = static_cast<double>( static_cast<unsigned char>( tr.at<uchar>(bi+i, bj+j) ) ) - 128.0;

            // D = P * A * P^T  (fast DCT via separable matrix)
            MatrixXd D = P * A * P.transpose();

            // Quantize: divide by Q and apply mask (bruit)
            MatrixXd Dq(8,8);
            for (int i=0;i<8;++i) for (int j=0;j<8;++j) Dq(i,j) = std::round(D(i,j) / Q(i,j));

            Dq = applyBruit(Dq, n);

            // copy back to out and count nonzero
            for (int i=0;i<8;++i) for (int j=0;j<8;++j) {
                out.at<double>(bi+i, bj+j) = Dq(i,j);
                if (Dq(i,j) != 0.0) ++nonzeroCount;
            }
        }
    }

    return {out, nonzeroCount};
}

// decompress single-channel: multiply by Q and inverse DCT P^T * (Dq * Q) * P
cv::Mat decompressChannel(const cv::Mat& compressed, const MatrixXd& P, const MatrixXd& Q) {
    int rows = compressed.rows, cols = compressed.cols;
    cv::Mat out = cv::Mat::zeros(rows, cols, CV_64F);

    for (int bi=0; bi<rows; bi += 8) {
        for (int bj=0; bj<cols; bj += 8) {
            MatrixXd Dq(8,8);
            for (int i=0;i<8;++i) for (int j=0;j<8;++j)
                Dq(i,j) = compressed.at<double>(bi+i, bj+j);

            // Multiply by quantization matrix to restore magnitude
            MatrixXd D_tilde = Dq.cwiseProduct(Q);

            // Inverse transform
            MatrixXd M = P.transpose() * D_tilde * P;

            // Copy block back
            for (int i=0;i<8;++i) for (int j=0;j<8;++j)
                out.at<double>(bi+i, bj+j) = M(i,j);
        }
    }

    return out;
}

int main(int argc, char** argv) {
    // Usage: ./codec image_path [n]
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <image_path> [n]\n";
        return 1;
    }
    string imgPath = argv[1];
    int n = 6; // default mask param
    if (argc >= 3) n = stoi(argv[2]);

    // Load image with OpenCV in BGR (uchar)
    cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
    if (img.empty()) {
        cerr << "Could not open image: " << imgPath << "\n";
        return 1;
    }

    // Convert from BGR to RGB for nicer save/display if desired (we'll keep BGR for OpenCV writing)
    cv::Mat img_tr = truncateToMultipleOf8(img);

    // Split channels (B,G,R)
    vector<cv::Mat> channels;
    cv::split(img_tr, channels);

    MatrixXd P = buildP();
    MatrixXd Q = buildQ();

    // compress each channel
    vector<cv::Mat> compressedChannels(3);
    long long totalNonZeroCompressed = 0;
    long long originalNonZero = 0;
    // count nonzero in original entire image (approx mimic to python approach)
    for (int c = 0; c < 3; ++c) {
        // each channel is CV_8U; count non-zero pixels (value != 0)
        originalNonZero += cv::countNonZero(channels[c]);
    }

    for (int c = 0; c < 3; ++c) {
        auto [comp, nonzeroCount] = compressChannel(channels[c], P, Q, n);
        compressedChannels[c] = comp;
        totalNonZeroCompressed += nonzeroCount;
    }

    // create composite compressed image (for visualization we keep DCT coefficients as doubles)
    // Now decompress
    vector<cv::Mat> decompressedChannels(3);
    for (int c = 0; c < 3; ++c) {
        cv::Mat decomp = decompressChannel(compressedChannels[c], P, Q);
        // add 128 back (undo centering)
        decomp = decomp + 128.0;
        decompressedChannels[c] = decomp;
    }

    // Merge and convert to 8-bit image for saving
    cv::Mat out;
    // Need to convert each channel from CV_64F to CV_8U with clipping
    vector<cv::Mat> out8(3);
    for (int c=0;c<3;++c) {
        cv::Mat tmp;
        decompressedChannels[c].convertTo(tmp, CV_8U); // this truncates; but we will clip manually for safety
        // Manual clipping to 0..255 to be safe:
        cv::threshold(tmp, tmp, 255, 255, cv::THRESH_TRUNC);
        // lower bound
        cv::Mat tooLow = (decompressedChannels[c] < 0.0);
        if (cv::countNonZero(tooLow) > 0) {
            // set low values to 0 in tmp
            for (int i=0;i<tmp.rows;++i) for (int j=0;j<tmp.cols;++j)
                if (decompressedChannels[c].at<double>(i,j) < 0.0) tmp.at<uchar>(i,j) = 0;
        }
        out8[c] = tmp;
    }
    cv::merge(out8, out);

    // Save results
    string outPath = "decompressed.png";
    if (!cv::imwrite(outPath, out)) {
        cerr << "Failed to write " << outPath << "\n";
    } else {
        cout << "Saved decompressed image to " << outPath << "\n";
    }

    // Calculate compression metric roughly analogous to original python:
    // python did: tauxCFinal = sum(nonzero per channel); tauxCFinal = tauxCFinal/((np.count_nonzero(ar))*3)
    // then used (1 - tauxCFinal)*100 as compression percent.
    double tauxCFinal = 0.0;
    if (originalNonZero == 0) originalNonZero = 1; // avoid divide by zero
    tauxCFinal = static_cast<double>(totalNonZeroCompressed) / (static_cast<double>(originalNonZero) * 3.0);
    double compressionPercent = (1.0 - tauxCFinal) * 100.0;

    cout << "n (mask threshold) = " << n << "\n";
    cout << "Total non-zero after compression (all channels): " << totalNonZeroCompressed << "\n";
    cout << "Original non-zero (sum over channels): " << originalNonZero << "\n";
    cout << "Approx compression percent (1 - taux) * 100 = " << compressionPercent << "%\n";

    // Optionally display using OpenCV windows (uncomment if you have a GUI)
    // cv::imshow("Original", img_tr);
    // cv::imshow("Decompressed", out);
    // cv::waitKey(0);

    return 0;
}
