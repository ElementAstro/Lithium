#ifndef LITHIUM_IMAGE_HIST_HPP
#define LITHIUM_IMAGE_HIST_HPP

#include <opencv2/core.hpp>
#include <vector>

std::vector<cv::Mat> CalHist(const cv::Mat& img);
cv::Mat CalGrayHist(const cv::Mat& img);

#endif