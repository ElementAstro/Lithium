#ifndef STRETCH_HPP
#define STRETCH_HPP

#include <opencv2/core.hpp>
#include <vector>

/**
 * @file stretch.hpp
 * @brief Image stretching and enhancement operations.
 * @details Provides various functions for image contrast stretching and
 * enhancement.
 */

/**
 * @brief Default minimum parameter for histogram stretching
 */
constexpr double DEFAULT_MIN_PARA = 0.0001;

/**
 * @brief Default maximum parameter for histogram stretching
 */
constexpr double DEFAULT_MAX_PARA = 0.0001;

/**
 * @brief Default black clip factor for gray stretching
 */
constexpr double DEFAULT_BLACK_CLIP = -1.25;

/**
 * @brief Default target background value
 */
constexpr double DEFAULT_TARGET_BKG = 0.1;

/**
 * @brief Small value to avoid division by zero
 */
constexpr double EPSILON = 1e-10;

/**
 * @brief Default kernel size for median blur operation
 */
constexpr int DEFAULT_MEDIAN_BLUR_SIZE = 3;

/**
 * @brief Parameters for image stretching operation
 */
struct StretchParams {
    double shadows;    /**< Shadow level (0.0 to 1.0) */
    double tones;      /**< Midtone level (0.0 to 1.0) */
    double highlights; /**< Highlight level (0.0 to 1.0) */
};

/**
 * @brief Performs white balance stretching on an image
 * @param hists Vector of histograms for each color channel
 * @param bgrPlanes Vector of BGR color planes
 * @return cv::Mat The white-balanced image
 * @throws std::invalid_argument if input vectors don't contain exactly 3
 * channels
 */
auto stretchWhiteBalance(const std::vector<cv::Mat>& hists,
                         const std::vector<cv::Mat>& bgrPlanes) -> cv::Mat;

/**
 * @brief Stretches a grayscale image using its histogram
 * @param hist Histogram of the grayscale image
 * @param plane Input grayscale image plane
 * @return cv::Mat The stretched grayscale image
 * @throws std::invalid_argument if input histogram or plane is empty
 */
auto stretchGray(const cv::Mat& hist, cv::Mat& plane) -> cv::Mat;

/**
 * @brief Performs gray level stretching with configurable parameters
 * @param img Input grayscale image
 * @param blackClip Black clipping factor (default: DEFAULT_BLACK_CLIP)
 * @param targetBkg Target background value (default: DEFAULT_TARGET_BKG)
 * @return cv::Mat The stretched grayscale image
 */
auto grayStretch(const cv::Mat& img, double blackClip = DEFAULT_BLACK_CLIP,
                 double targetBkg = DEFAULT_TARGET_BKG) -> cv::Mat;

/**
 * @brief Stretches a single channel using provided parameters
 * @param normalizedImg Normalized input image (0.0 to 1.0)
 * @param params Stretching parameters
 * @return cv::Mat The stretched channel
 */
auto stretchOneChannel(const cv::Mat& normalizedImg,
                       const StretchParams& params) -> cv::Mat;

/**
 * @brief Stretches all three channels of an image independently
 * @param img Input BGR image
 * @param shadows Shadow levels for each channel
 * @param midtones Midtone levels for each channel
 * @param highlights Highlight levels for each channel
 * @param inputRange Input image range (e.g., 255 for 8-bit)
 * @param doJpg Whether to output 8-bit (true) or 16-bit (false)
 * @return cv::Mat The stretched color image
 * @throws std::invalid_argument if input vectors don't match channel count
 */
auto stretchThreeChannels(const cv::Mat& img,
                          const std::vector<double>& shadows,
                          const std::vector<double>& midtones,
                          const std::vector<double>& highlights, int inputRange,
                          bool doJpg = false) -> cv::Mat;

/**
 * @brief Calculates optimal stretch parameters for an image
 * @param img Input image
 * @return std::tuple<double,double,double> Tuple of shadows, midtones, and
 * highlights
 */
auto calculateStretchParameters(const cv::Mat& img)
    -> std::tuple<double, double, double>;

/**
 * @brief Performs automatic stretching based on image content
 * @param img Input image (grayscale or color)
 * @return cv::Mat The automatically stretched image
 */
auto autoStretch(const cv::Mat& img) -> cv::Mat;

/**
 * @brief Performs adaptive local stretching using block processing
 * @param img Input image
 * @param blockSize Size of local processing blocks (default: 16)
 * @return cv::Mat The adaptively stretched image
 * @throws std::invalid_argument if blockSize is less than 1
 */
auto adaptiveStretch(const cv::Mat& img, int blockSize = 16) -> cv::Mat;

#endif  // STRETCH_HPP