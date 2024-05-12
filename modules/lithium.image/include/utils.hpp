#ifndef LITHIUM_IMAGE_UTILS_HPP
#define LITHIUM_IMAGE_UTILS_HPP

#include <opencv2/core.hpp>
#include <string>
#include <tuple>
#include <vector>

std::vector<cv::Mat> loadImages(const std::string& folder,
                                const std::vector<std::string>& filenames);
bool insideCircle(int x, int y, int centerX, int centerY, float radius);
bool checkElongated(int width, int height);
int checkWhitePixel(const cv::Mat& rect_contour, int x, int y);
int EightSymmetryCircleCheck(const cv::Mat& rect_contour,
                             const cv::Point& center, int x_p, int y_p);
int FourSymmetryCircleCheck(const cv::Mat& rect_contour,
                            const cv::Point& center, float radius);
std::tuple<int, std::vector<int>, std::vector<double>> define_narrow_radius(
    int min_area, double max_area, double area, double scale);
bool BresenHamCheckCircle(const cv::Mat& rect_contour, float radius,
                          float pixelratio, bool if_debug = false);
double Cal_Avgdev(double mid, const cv::Mat& norm_img);
cv::Mat MTF(double m, const cv::Mat& img);
double CalScale(const cv::Mat& img, int resize_size = 1552);
double Cal_Middev(double mid, const cv::Mat& img);
#endif