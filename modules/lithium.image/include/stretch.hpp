#ifndef LITHIUM_IMAGE_STRETCH_HPP
#define LITHIUM_IMAGE_STRETCH_HPP

#include <opencv2/core.hpp>
#include <vector>

cv::Mat Stretch_WhiteBalance(const std::vector<cv::Mat>& hists,
                             const std::vector<cv::Mat>& bgr_planes);
cv::Mat StretchGray(const cv::Mat& hist, cv::Mat& plane);
#endif