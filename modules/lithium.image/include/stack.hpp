#ifndef LITHIUN_IMAGE_STACK_HPP
#define LITHIUN_IMAGE_STACK_HPP

#include <opencv2/core.hpp>

// 叠加方式
enum StackMode {
    MEAN,
    MEDIAN,
    MAXIMUM,
    MINIMUM,
    SIGMA_CLIPPING,
    WEIGHTED_MEAN,
    LIGHTEN
};

cv::Mat stackImages(const std::vector<cv::Mat>& images, StackMode mode,
                    float sigma = 2.0) ;

#endif

