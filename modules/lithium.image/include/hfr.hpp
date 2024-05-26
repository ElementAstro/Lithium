#ifndef LITHIUM_IMAGE_HFR_HPP
#define LITHIUM_IMAGE_HFR_HPP

#include <opencv2/core.hpp>

#include "atom/type/json.hpp"
using json = nlohmann::json;

double calcHfr(const cv::Mat& inImage, float radius);

std::tuple<cv::Mat, int, double, json> StarDetectAndHfr(
    const cv::Mat& img, bool if_removehotpixel, bool if_noiseremoval,
    bool do_star_mark = false, bool down_sample_mean_std = true,
    cv::Mat mark_img = cv::Mat());

#endif
