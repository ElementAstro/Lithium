#include "StarDetector.h"
#include <algorithm>

StarDetector::StarDetector(int maxStars) : maxStars(maxStars) {}

std::vector<StarDetector::Star> StarDetector::detectStars(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }

    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<Star> stars;
    for (const auto& contour : contours) {
        if (contour.size() >= 5) {
            cv::RotatedRect ellipse = cv::fitEllipse(contour);
            Star star;
            star.center = ellipse.center;
            star.hfr = (ellipse.size.width + ellipse.size.height) / 4.0;
            stars.push_back(star);
        }
    }

    std::sort(stars.begin(), stars.end(), [&gray](const Star& a, const Star& b) {
        return gray.at<uchar>(a.center) > gray.at<uchar>(b.center);
    });
    if (stars.size() > maxStars) {
        stars.resize(maxStars);
    }

    return stars;
}
