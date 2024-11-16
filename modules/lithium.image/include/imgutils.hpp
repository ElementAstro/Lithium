#ifndef LITHIUM_IMAGE_UTILS_HPP
#define LITHIUM_IMAGE_UTILS_HPP

#include <opencv2/core.hpp>
#include <tuple>
#include <vector>


auto insideCircle(int xCoord, int yCoord, int centerX, int centerY,
                  float radius) -> bool;
auto checkElongated(int width, int height) -> bool;
auto checkWhitePixel(const cv::Mat& rect_contour, int x_coord,
                     int y_coord) -> int;
auto checkEightSymmetryCircle(const cv::Mat& rect_contour,
                              const cv::Point& center, int x_p, int y_p) -> int;
auto checkFourSymmetryCircle(const cv::Mat& rect_contour,
                             const cv::Point& center, float radius) -> int;
auto defineNarrowRadius(int min_area, double max_area, double area,
                        double scale)
    -> std::tuple<int, std::vector<int>, std::vector<double>>;
auto checkBresenhamCircle(const cv::Mat& rect_contour, float radius,
                          float pixel_ratio, bool if_debug = false) -> bool;
auto calculateAverageDeviation(double mid, const cv::Mat& norm_img) -> double;
auto calculateMTF(double magnitude, const cv::Mat& img) -> cv::Mat;
auto calculateScale(const cv::Mat& img, int resize_size = 1552) -> double;
auto calculateMedianDeviation(double mid, const cv::Mat& img) -> double;
auto computeParamsOneChannel(const cv::Mat& img)
    -> std::tuple<double, double, double>;
auto autoWhiteBalance(const cv::Mat& img) -> cv::Mat;

#endif  // LITHIUM_IMAGE_UTILS_HPP