#ifndef DCT_COMPRESSOR_H
#define DCT_COMPRESSOR_H

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>

class DCTCompressor {
private:
    Eigen::MatrixXd P;
    Eigen::MatrixXd Q;
    int n; // Paramètre pour le masquage des hautes fréquences

public:
    DCTCompressor(int nValue = 6);

    cv::Mat truncateToMultipleOf8(const cv::Mat& img);
    cv::Mat decompressChannel(const cv::Mat& compressed);
    std::pair<cv::Mat, long long> compressChannel(const cv::Mat& channel);

    Eigen::MatrixXd buildP();
    Eigen::MatrixXd buildQ();
    Eigen::MatrixXd applyBruit(const Eigen::MatrixXd& block);

    void setN(int nValue) { n = nValue; }
    int getN() const { return n; }
};

#endif
