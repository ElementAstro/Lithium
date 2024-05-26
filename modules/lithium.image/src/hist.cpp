#include "hist.hpp"

#include <cmath>
#include <opencv2/imgproc.hpp>

std::vector<cv::Mat> CalHist(const cv::Mat& img) {
    std::vector<cv::Mat> bgr_planes;
    cv::split(img, bgr_planes);

    int histSize = 65535;
    float range[] = {0, 65535};
    const float* histRange = {range};
    bool accumulate = false;

    std::vector<cv::Mat> histograms;
    for (int i = 0; i < 3; ++i) {
        cv::Mat hist;
        cv::calcHist(&bgr_planes[i], 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, accumulate);
        cv::threshold(hist, hist, 4, 0, cv::THRESH_TOZERO);
        histograms.push_back(hist);
    }

    return histograms;
}

cv::Mat CalGrayHist(const cv::Mat& img) {
    int histSize = 65535;
    float range[] = {0, 65535};
    const float* histRange = {range};
    bool accumulate = false;

    cv::Mat gray_hist;
    cv::calcHist(&img, 1, 0, cv::Mat(), gray_hist, 1, &histSize, &histRange, accumulate);
    cv::threshold(gray_hist, gray_hist, 1, 0, cv::THRESH_TOZERO);

    return gray_hist;
}
