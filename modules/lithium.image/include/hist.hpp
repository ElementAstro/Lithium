#ifndef HIST_HPP
#define HIST_HPP

#include <opencv2/core.hpp>
#include <vector>

constexpr int DEFAULT_HIST_SIZE = 65535;
constexpr int DEFAULT_WIDTH = 512;
constexpr int DEFAULT_HEIGHT = 400;
constexpr int DEFAULT_LINE_TYPE = 8;
constexpr int DEFAULT_COLOR_VALUE = 255;

auto calculateHist(const cv::Mat& img, int histSize = DEFAULT_HIST_SIZE,
                   bool normalize = false) -> std::vector<cv::Mat>;
auto calculateGrayHist(const cv::Mat& img, int histSize = DEFAULT_HIST_SIZE,
                       bool normalize = false) -> cv::Mat;
auto calculateCDF(const cv::Mat& hist) -> cv::Mat;
auto equalizeHistogram(const cv::Mat& img) -> cv::Mat;
auto drawHistogram(const cv::Mat& hist, int histSize = DEFAULT_HIST_SIZE,
                   int width = DEFAULT_WIDTH,
                   int height = DEFAULT_HEIGHT) -> cv::Mat;

#endif  // HIST_HPP