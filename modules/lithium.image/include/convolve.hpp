#ifndef LITHIUM_IMAGE_CONVOLVE_HPP
#define LITHIUM_IMAGE_CONVOLVE_HPP

#include <opencv2/core.hpp>

void convolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output);
void dftConvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output);
void deconvolve(const cv::Mat& input, const cv::Mat& kernel, cv::Mat& output);
void separableConvolve(const cv::Mat& input, const cv::Mat& kernelX,
                       const cv::Mat& kernelY, cv::Mat& output);

#endif
