#ifndef LITHIUM_IMAGE_CENTROID_HPP
#define LITHIUM_IMAGE_CENTROID_HPP

#include <opencv2/opencv.hpp>
#include <string>

#include "atom/macro.hpp"

constexpr float DEFAULT_EPSILON = 1e-6F;
constexpr int MAX_ITERATIONS_DEFAULT = 10;

class StarCentroid {
public:
    // 结构体32字节对齐
    struct CentroidResult {
        cv::Point2f weightedCenter;
        cv::Point2f subPixelCenter;
        cv::Point2i roundedCenter;
    } ATOM_ALIGNAS(32);

    static auto readFits(const std::string& filename) -> cv::Mat;

    static auto calcIntensityWeightedCenter(const cv::Mat& image) -> cv::Point2f ;

    static auto calcSubPixelCenter(const cv::Mat& roi,
                               cv::Point2f&& initCenter,
                               float epsilon = DEFAULT_EPSILON,
                               int maxIterations = MAX_ITERATIONS_DEFAULT) -> cv::Point2f;

    static auto findCentroid(const cv::Mat& image) -> CentroidResult;

    static void visualizeResults(const cv::Mat& image,
                             const CentroidResult& result);
};

#endif  // LITHIUM_IMAGE_CENTROID_HPP