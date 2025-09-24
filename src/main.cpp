#include <iostream>
#include "DCT.h"
#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <image_path> [n]\n";
        return 1;
    }

    std::string imgPath = argv[1];
    int n = (argc >= 3) ? std::stoi(argv[2]) : 6;

    cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Could not open image: " << imgPath << "\n";
        return 1;
    }

    DCTCompressor compressor(n);
    cv::Mat img_tr = compressor.truncateToMultipleOf8(img);

    std::vector<cv::Mat> channels;
    cv::split(img_tr, channels);

    std::vector<cv::Mat> compressedChannels(3);
    long long totalNonZeroCompressed = 0, originalNonZero = 0;

    for (int c=0; c<3; ++c) {
        originalNonZero += cv::countNonZero(channels[c]);
        auto [comp, count] = compressor.compressChannel(channels[c]);
        compressedChannels[c] = comp;
        totalNonZeroCompressed += count;
    }

    std::vector<cv::Mat> decompressedChannels(3);
    for (int c=0; c<3; ++c) {
        cv::Mat decomp = compressor.decompressChannel(compressedChannels[c]) + 128.0;
        decompressedChannels[c] = decomp;
    }

    std::vector<cv::Mat> out8(3);
    for (int c=0;c<3;++c) {
        cv::Mat tmp;
        decompressedChannels[c].convertTo(tmp, CV_8U);
        out8[c] = tmp;
    }

    cv::Mat out;
    cv::merge(out8, out);
    cv::imwrite("decompressed.png", out);

    double tauxCFinal = static_cast<double>(totalNonZeroCompressed) / (originalNonZero*1.0);
    double compressionPercent = (1.0 - tauxCFinal) * 100.0;

    std::cout << "n = " << n << "\nCompression ≈ " << compressionPercent << "%\n";
    return 0;
}
