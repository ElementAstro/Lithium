#ifndef BINNING_H
#define BINNING_H

#include <opencv2/opencv.hpp>
#include <span>
#include <boost/iterator/counting_iterator.hpp>

struct CamBin {
    uint32_t camxbin{1};
    uint32_t camybin{1};
};

CamBin mergeImageBasedOnSize(const cv::Mat& image);

cv::Mat processMatWithBinAvg(const cv::Mat& image, uint32_t camxbin, uint32_t camybin, bool isColor = false, bool isAVG = true);

template <typename T>
void parallel_process_bin(std::span<const uint8_t> srcData, cv::Mat& result, uint32_t width, uint32_t height, uint32_t camxbin, uint32_t camybin, uint32_t binArea);

template <typename T>
void process_mono_bin(std::span<const uint8_t> srcData, cv::Mat& result, uint32_t srcStride, uint32_t camxbin, uint32_t camybin);

template <typename T>
T calculateAverage(std::span<const T> values, size_t binSize);

cv::Mat processWithAverage(std::span<const uint8_t> srcData, uint32_t width, uint32_t height, uint32_t depth, uint32_t newWidth, uint32_t newHeight, uint32_t camxbin, uint32_t camybin);

cv::Mat processWithBinning(std::span<const uint8_t> srcData, uint32_t width, uint32_t height, uint32_t channels, uint32_t depth, uint32_t newWidth, uint32_t newHeight, uint32_t camxbin, uint32_t camybin, bool isColor);

#endif // BINNING_H