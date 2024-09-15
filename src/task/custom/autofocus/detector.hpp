#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

class StarDetector {
public:
    struct Star {
        cv::Point2f center;
        double hfr;
    };

    StarDetector(int maxStars = 10);
    std::vector<Star> detectStars(const cv::Mat& image);

private:
    int maxStars;
};
