#include "hist.hpp"

#include <cmath>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

auto calculateHist(const cv::Mat& img, int histSize,
                   bool normalize) -> std::vector<cv::Mat> {
    LOG_F(INFO, "Calculating BGR histograms with histSize: {}", histSize);
    if (img.empty()) {
        LOG_F(ERROR, "Input image for calculateHist is empty.");
        THROW_INVALID_ARGUMENT("Input image for calculateHist is empty.");
    }
    if (img.channels() != 3) {
        LOG_F(ERROR, "Input image does not have 3 channels.");
        THROW_INVALID_ARGUMENT("Input image does not have 3 channels.");
    }

    std::vector<cv::Mat> bgrPlanes;
    cv::split(img, bgrPlanes);

    std::array<float, 2> range = {0, static_cast<float>(histSize)};
    const float* histRange = range.data();
    bool accumulate = false;

    std::vector<cv::Mat> histograms;
    histograms.reserve(3);

    for (int i = 0; i < 3; ++i) {
        cv::Mat hist;
        cv::calcHist(&bgrPlanes[i], 1, 0, cv::Mat(), hist, 1, &histSize,
                     &histRange, accumulate);
        cv::threshold(hist, hist, 4, 0, cv::THRESH_TOZERO);
        if (normalize) {
            cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX);
            LOG_F(INFO, "Normalized histogram for channel {}", i);
        }
        histograms.push_back(hist);
        LOG_F(INFO, "Calculated histogram for channel {}", i);
    }

    LOG_F(INFO, "Completed BGR histogram calculation.");
    return histograms;
}

auto calculateGrayHist(const cv::Mat& img, int histSize,
                       bool normalize) -> cv::Mat {
    LOG_F(INFO, "Calculating grayscale histogram with histSize: {}", histSize);
    if (img.empty()) {
        LOG_F(ERROR, "Input image for calculateGrayHist is empty.");
        THROW_INVALID_ARGUMENT("Input image for calculateGrayHist is empty.");
    }
    if (img.channels() != 1) {
        LOG_F(ERROR, "Input image is not grayscale.");
        THROW_INVALID_ARGUMENT("Input image is not grayscale.");
    }

    std::array<float, 2> range = {0, static_cast<float>(histSize)};
    const float* histRange = range.data();
    bool accumulate = false;

    cv::Mat grayHist;
    cv::calcHist(&img, 1, 0, cv::Mat(), grayHist, 1, &histSize, &histRange,
                 accumulate);
    cv::threshold(grayHist, grayHist, 1, 0, cv::THRESH_TOZERO);

    if (normalize) {
        cv::normalize(grayHist, grayHist, 0, 1, cv::NORM_MINMAX);
        LOG_F(INFO, "Normalized grayscale histogram.");
    }

    LOG_F(INFO, "Completed grayscale histogram calculation.");
    return grayHist;
}

auto calculateCDF(const cv::Mat& hist) -> cv::Mat {
    LOG_F(INFO, "Calculating CDF.");
    if (hist.empty()) {
        LOG_F(ERROR, "Input histogram for calculateCDF is empty.");
        THROW_INVALID_ARGUMENT("Input histogram for calculateCDF is empty.");
    }

    cv::Mat cdf;
    hist.copyTo(cdf);
    for (int i = 1; i < hist.rows; ++i) {
        cdf.at<float>(i) += cdf.at<float>(i - 1);
    }
    cv::normalize(cdf, cdf, 0, 1, cv::NORM_MINMAX);
    LOG_F(INFO, "Completed CDF calculation.");
    return cdf;
}

auto equalizeHistogram(const cv::Mat& img) -> cv::Mat {
    LOG_F(INFO, "Starting histogram equalization.");
    if (img.empty()) {
        LOG_F(ERROR, "Input image for equalizeHistogram is empty.");
        THROW_INVALID_ARGUMENT("Input image for equalizeHistogram is empty.");
    }

    cv::Mat equalized;
    if (img.channels() == 1) {
        cv::equalizeHist(img, equalized);
    } else {
        std::vector<cv::Mat> bgrPlanes;
        cv::split(img, bgrPlanes);
        for (auto& plane : bgrPlanes) {
            cv::equalizeHist(plane, plane);
        }
        cv::merge(bgrPlanes, equalized);
    }
    LOG_F(INFO, "Completed histogram equalization.");
    return equalized;
}

auto drawHistogram(const cv::Mat& hist, int histSize, int width,
                   int height) -> cv::Mat {
    LOG_F(INFO, "Drawing histogram.");
    if (hist.empty()) {
        LOG_F(ERROR, "Input histogram for drawHistogram is empty.");
        THROW_INVALID_ARGUMENT("Input histogram for drawHistogram is empty.");
    }
    cv::Mat histImage(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    // Normalize the histogram to fit the image height
    cv::Mat histNorm;
    cv::normalize(hist, histNorm, 0, histImage.rows, cv::NORM_MINMAX);

    int binWidth = cvRound(static_cast<double>(width) / histSize);

    for (int i = 1; i < histSize; ++i) {
        cv::line(
            histImage,
            cv::Point(binWidth * (i - 1),
                      height - cvRound(histNorm.at<float>(i - 1))),
            cv::Point(binWidth * i, height - cvRound(histNorm.at<float>(i))),
            cv::Scalar(DEFAULT_COLOR_VALUE, 0, 0), 2, DEFAULT_LINE_TYPE, 0);
    }

    LOG_F(INFO, "Completed drawing histogram.");
    return histImage;
}