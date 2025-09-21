#include "DCT.h"
#include <cmath>

using Eigen::MatrixXd;

DCTCompressor::DCTCompressor(int nValue) : n(nValue) {
    P = buildP();
    Q = buildQ();
}

MatrixXd DCTCompressor::buildP() {
    MatrixXd P(8,8);
    for (int k=0; k<8; ++k) {
        for (int i=0; i<8; ++i) {
            double Ck = (k==0) ? std::sqrt(1.0/2.0) : 1.0;
            P(k,i) = std::sqrt(2.0/8.0) * Ck * std::cos((M_PI/(2.0*8.0)) * (2.0*i + 1.0) * k);
        }
    }
    return P;
}

MatrixXd DCTCompressor::buildQ() {
    static const double Q_data[8][8] = {
        {16,11,10,16,24,40,51,61},
        {12,12,14,19,26,58,60,55},
        {14,13,16,24,40,57,69,56},
        {14,17,22,29,51,87,80,62},
        {18,22,37,56,68,109,103,77},
        {24,35,55,64,81,104,113,92},
        {49,64,78,87,103,121,120,101},
        {72,92,95,98,112,100,103,99}
    };
    MatrixXd Q(8,8);
    for (int i=0;i<8;++i)
        for (int j=0;j<8;++j)
            Q(i,j) = Q_data[i][j];
    return Q;
}

cv::Mat DCTCompressor::truncateToMultipleOf8(const cv::Mat& img) {
    int x = img.rows / 8 * 8;
    int y = img.cols / 8 * 8;
    return img(cv::Range(0, x), cv::Range(0, y)).clone();
}

MatrixXd DCTCompressor::applyBruit(const MatrixXd& block) {
    MatrixXd M = block;
    for (int i=0; i<8; ++i)
        for (int j=0; j<8; ++j)
            if ((i+j) > n) M(i,j) = 0.0;
    return M;
}

std::pair<cv::Mat, long long> DCTCompressor::compressChannel(const cv::Mat& channel) {
    cv::Mat tr = truncateToMultipleOf8(channel);
    int rows = tr.rows, cols = tr.cols;
    cv::Mat out = cv::Mat::zeros(rows, cols, CV_64F);
    long long nonzeroCount = 0;

    for (int bi=0; bi<rows; bi+=8) {
        for (int bj=0; bj<cols; bj+=8) {
            MatrixXd A(8,8);
            for (int i=0;i<8;++i)
                for (int j=0;j<8;++j)
                    A(i,j) = static_cast<double>(tr.at<uchar>(bi+i,bj+j)) - 128.0;

            MatrixXd D = P * A * P.transpose();
            MatrixXd Dq(8,8);
            for (int i=0;i<8;++i)
                for (int j=0;j<8;++j)
                    Dq(i,j) = std::round(D(i,j)/Q(i,j));

            Dq = applyBruit(Dq);

            for (int i=0;i<8;++i)
                for (int j=0;j<8;++j) {
                    out.at<double>(bi+i,bj+j) = Dq(i,j);
                    if (Dq(i,j)!=0.0) ++nonzeroCount;
                }
        }
    }
    return {out, nonzeroCount};
}

cv::Mat DCTCompressor::decompressChannel(const cv::Mat& compressed) {
    int rows = compressed.rows, cols = compressed.cols;
    cv::Mat out = cv::Mat::zeros(rows, cols, CV_64F);

    for (int bi=0; bi<rows; bi+=8) {
        for (int bj=0; bj<cols; bj+=8) {
            MatrixXd Dq(8,8);
            for (int i=0;i<8;++i)
                for (int j=0;j<8;++j)
                    Dq(i,j) = compressed.at<double>(bi+i,bj+j);

            MatrixXd D_tilde = Dq.cwiseProduct(Q);
            MatrixXd M = P.transpose() * D_tilde * P;

            for (int i=0;i<8;++i)
                for (int j=0;j<8;++j)
                    out.at<double>(bi+i,bj+j) = M(i,j);
        }
    }
    return out;
}
