#include "imgutils.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

constexpr double MIN_LONG_RATIO = 1.5;
constexpr int MAX_SAMPLES = 500000;
constexpr double MAGIC_1_4826 = 1.4826;
constexpr double MAGIC_2_8 = 2.8;
constexpr double BASE_RATIO = 0.25;  // Renamed from B to BASE_RATIO

auto insideCircle(int xCoord, int yCoord, int centerX, int centerY,
                  float radius) -> bool {
    LOG_F(INFO,
          "Checking if point ({}, {}) is inside circle with center ({}, {}) "
          "and radius {}",
          xCoord, yCoord, centerX, centerY, radius);
    return std::sqrt((xCoord - centerX) * (xCoord - centerX) +
                     (yCoord - centerY) * (yCoord - centerY)) < radius;
}

auto checkElongated(int width, int height) -> bool {
    LOG_F(INFO, "Checking elongation for width: {}, height: {}", width, height);
    double ratio = width > height ? static_cast<double>(width) / height
                                  : static_cast<double>(height) / width;
    bool elongated = ratio > MIN_LONG_RATIO;
    LOG_F(INFO, "Elongated: {}", elongated);
    return elongated;
}

auto checkWhitePixel(const cv::Mat& rect_contour, int xCoord,
                     int yCoord) -> int {
    LOG_F(INFO, "Checking white pixel at ({}, {})", xCoord, yCoord);
    if (xCoord >= 0 && xCoord < rect_contour.cols && yCoord >= 0 &&
        yCoord < rect_contour.rows) {
        try {
            return rect_contour.at<uint16_t>(yCoord, xCoord) > 0 ? 1 : 0;
        } catch (const cv::Exception& e) {
            LOG_F(ERROR, "Exception accessing pixel at ({}, {}): {}", xCoord,
                  yCoord, e.what());
            return 0;
        }
    }
    LOG_F(WARNING, "Pixel coordinates ({}, {}) out of bounds", xCoord, yCoord);
    return 0;
}

auto eightSymmetryCircleCheck(const cv::Mat& rect_contour,
                              const cv::Point& center, int xCoord,
                              int yCoord) -> int {
    LOG_F(INFO,
          "Performing EightSymmetryCircleCheck with xCoord: {}, yCoord: {}",
          xCoord, yCoord);
    int whitePixelCount = 0;
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x + xCoord, center.y + yCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x - xCoord, center.y + yCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x + xCoord, center.y - yCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x - xCoord, center.y - yCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x + yCoord, center.y + xCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x + yCoord, center.y - xCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x - yCoord, center.y + xCoord);
    whitePixelCount +=
        checkWhitePixel(rect_contour, center.x - yCoord, center.y - xCoord);
    LOG_F(INFO, "White pixel count after symmetry check: {}", whitePixelCount);
    return whitePixelCount;
}

auto fourSymmetryCircleCheck(const cv::Mat& rect_contour,
                             const cv::Point& center, float radius) -> int {
    LOG_F(INFO, "Performing FourSymmetryCircleCheck with radius: {}", radius);
    int whitePixelCount = 0;
    whitePixelCount += checkWhitePixel(rect_contour, center.x,
                                       center.y + static_cast<int>(radius));
    whitePixelCount += checkWhitePixel(rect_contour, center.x,
                                       center.y - static_cast<int>(radius));
    whitePixelCount += checkWhitePixel(
        rect_contour, center.x - static_cast<int>(radius), center.y);
    whitePixelCount += checkWhitePixel(
        rect_contour, center.x + static_cast<int>(radius), center.y);
    LOG_F(INFO, "White pixel count after four symmetry check: {}",
          whitePixelCount);
    return whitePixelCount;
}

auto defineNarrowRadius(int minArea, double maxArea, double area, double scale)
    -> std::tuple<int, std::vector<int>, std::vector<double>> {
    LOG_F(INFO,
          "Defining narrow radius with minArea: {}, maxArea: {}, area: {}, "
          "scale: {}",
          minArea, maxArea, area, scale);
    std::vector<int> checklist;
    std::vector<double> thresholdList;
    int checkNum = 0;

    constexpr int AREA_THRESHOLD_1 = 500;
    constexpr int AREA_THRESHOLD_2 = 1000;
    constexpr double THRESHOLD_1 = 0.5;
    constexpr double THRESHOLD_2 = 0.65;
    constexpr double THRESHOLD_3 = 0.75;

    if (minArea <= area && area <= AREA_THRESHOLD_1 * scale) {
        checkNum = 2;
        checklist = {1, 2};
        thresholdList = {THRESHOLD_1, THRESHOLD_2};
    } else if (AREA_THRESHOLD_1 * scale < area &&
               area <= AREA_THRESHOLD_2 * scale) {
        checkNum = 3;
        checklist = {2, 3, 4};
        thresholdList = {THRESHOLD_1, THRESHOLD_2, THRESHOLD_3};
    } else if (AREA_THRESHOLD_2 * scale < area && area <= maxArea) {
        checkNum = 3;
        checklist = {2, 3, 4};
        thresholdList = {THRESHOLD_1, THRESHOLD_2, THRESHOLD_3};
    } else {
        checkNum = 0;
        checklist = {};
        thresholdList = {};
        LOG_F(WARNING, "Area {} is out of defined thresholds.", area);
    }
    LOG_F(INFO,
          "defineNarrowRadius result - checkNum: {}, checklist size: {}, "
          "thresholdList size: {}",
          checkNum, checklist.size(), thresholdList.size());
    return {checkNum, checklist, thresholdList};
}

auto checkBresenhamCircle(const cv::Mat& rect_contour, float radius,
                          float pixelRatio, bool ifDebug) -> bool {
    LOG_F(INFO,
          "Starting BresenhamCircleCheck with radius: {}, pixelRatio: {}, "
          "ifDebug: {}",
          radius, pixelRatio, ifDebug);
    cv::Mat rectContourRgb;
    if (ifDebug) {
        cv::cvtColor(rect_contour, rectContourRgb, cv::COLOR_GRAY2BGR);
        LOG_F(INFO, "Converted rect_contour to RGB for debugging.");
    }

    int totalPixelCount = 0;
    int whitePixelCount = 0;

    cv::Size shape = rect_contour.size();
    cv::Point center(shape.width / 2, shape.height / 2);

    int p = 1 - static_cast<int>(radius);
    int xCoord = 0;
    int yCoord = static_cast<int>(radius);
    whitePixelCount += fourSymmetryCircleCheck(rect_contour, center, radius);
    totalPixelCount += 4;

    while (xCoord <= yCoord) {
        xCoord += 1;
        if (p < 0) {
            p += 2 * xCoord + 1;
        } else {
            yCoord -= 1;
            p += 2 * (xCoord - yCoord) + 1;
        }

        if (ifDebug) {
            // Future implementation for debugging can be added here
            LOG_F(INFO, "Debug mode: xCoord = {}, yCoord = {}", xCoord, yCoord);
        } else {
            whitePixelCount +=
                eightSymmetryCircleCheck(rect_contour, center, xCoord, yCoord);
        }

        totalPixelCount += 8;
    }

    float ratio = static_cast<float>(whitePixelCount) / totalPixelCount;
    LOG_F(INFO, "BresenhamCircleCheck ratio: {}", ratio);

    if (ifDebug) {
        std::cout << "ratio: " << ratio << std::endl;
    }

    bool result = ratio > pixelRatio;
    LOG_F(INFO, "BresenhamCircleCheck result: {}", result);
    return result;
}

auto calculateAverageDeviation(double mid, const cv::Mat& norm_img) -> double {
    LOG_F(INFO, "Calculating average deviation with mid: {}", mid);
    if (norm_img.empty()) {
        LOG_F(ERROR, "normalize image is empty.");
        THROW_INVALID_ARGUMENT("normalize image is empty.");
    }
    int size = norm_img.rows * norm_img.cols;
    double sum = 0;
    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            sum += std::abs(norm_img.at<double>(i, j) - mid);
        }
    }
    double avgDev = sum / size;
    LOG_F(INFO, "Average deviation: {}", avgDev);
    return avgDev;
}

auto calculateMTF(double mean, const cv::Mat& img) -> cv::Mat {
    LOG_F(INFO, "Calculating MTF with mean: {}", mean);
    if (img.empty()) {
        LOG_F(ERROR, "Input image for MTF is empty.");
        THROW_INVALID_ARGUMENT("Input image for MTF is empty.");
    }
    cv::Mat result = img.clone();
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            double value = img.at<double>(i, j);
            if (value != 0 && value != mean && value != 1) {
                double denominator = ((2 * mean - 1) * value) - mean;
                if (denominator != 0) {
                    result.at<double>(i, j) = (mean - 1) * value / denominator;
                } else {
                    LOG_F(WARNING,
                          "Denominator is zero at ({}, {}), skipping MTF "
                          "calculation.",
                          i, j);
                }
            }
        }
    }
    LOG_F(INFO, "Completed MTF calculation.");
    return result;
}

auto calculateScale(const cv::Mat& img, int resize_size) -> double {
    LOG_F(INFO, "Calculating scale with resize_size: {}", resize_size);
    if (img.empty()) {
        LOG_F(ERROR, "Input image for scale calculation is empty.");
        THROW_INVALID_ARGUMENT("Input image for scale calculation is empty.");
    }
    double scale = (img.rows > img.cols)
                       ? static_cast<double>(resize_size) / img.rows
                       : static_cast<double>(resize_size) / img.cols;
    LOG_F(INFO, "Calculated scale: {}", scale);
    return scale;
}

auto calculateMedianDeviation(double mid, const cv::Mat& img) -> double {
    LOG_F(INFO, "Calculating median deviation with mid: {}", mid);
    if (img.empty()) {
        LOG_F(ERROR, "Input image for median deviation is empty.");
        THROW_INVALID_ARGUMENT("Input image for median deviation is empty.");
    }
    std::vector<double> deviations;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            deviations.push_back(std::abs(img.at<double>(i, j) - mid));
        }
    }
    if (deviations.empty()) {
        LOG_F(WARNING, "No deviations found in image.");
        return 0.0;
    }
    std::nth_element(deviations.begin(),
                     deviations.begin() + deviations.size() / 2,
                     deviations.end());
    double medianDeviation = deviations[deviations.size() / 2];
    LOG_F(INFO, "Median deviation: {}", medianDeviation);
    return medianDeviation;
}

auto computeParamsOneChannel(const cv::Mat& img)
    -> std::tuple<double, double, double> {
    LOG_F(INFO, "Computing parameters for one channel.");
    if (img.empty()) {
        LOG_F(ERROR, "Input image for computeParamsOneChannel is empty.");
        THROW_INVALID_ARGUMENT(
            "Input image for computeParamsOneChannel is empty.");
    }

    // Flatten the image and sample it
    std::vector<uchar> bufferValue;
    if (img.isContinuous()) {
        bufferValue.assign(img.data, img.data + img.total());
    } else {
        for (int i = 0; i < img.rows; ++i) {
            bufferValue.insert(bufferValue.end(), img.ptr<uchar>(i),
                               img.ptr<uchar>(i) + img.cols);
        }
    }

    int sampleBy = (img.rows * img.cols < MAX_SAMPLES)
                       ? 1
                       : (img.rows * img.cols / MAX_SAMPLES);

    std::vector<uchar> sampleValue;
    for (size_t i = 0; i < bufferValue.size(); i += sampleBy) {
        sampleValue.push_back(bufferValue[i]);
    }

    if (sampleValue.empty()) {
        LOG_F(WARNING, "Sampled values are empty.");
        return {0.0, 0.0, 0.0};
    }

    // Compute median of sampled values
    size_t n = sampleValue.size() / 2;
    std::nth_element(sampleValue.begin(), sampleValue.begin() + n,
                     sampleValue.end());
    double medianSample = sampleValue[n];
    LOG_F(INFO, "Median sample: {}", medianSample);

    // Compute Median Absolute Deviation (MAD)
    std::vector<double> absDev(sampleValue.size());
    std::transform(
        sampleValue.begin(), sampleValue.end(), absDev.begin(),
        [medianSample](uchar value) {
            return std::abs(static_cast<double>(value) - medianSample);
        });

    std::nth_element(absDev.begin(), absDev.begin() + n, absDev.end());
    double medDev = absDev[n];
    LOG_F(INFO, "Median Absolute Deviation: {}", medDev);

    // Normalize
    double inputRange = img.depth() == CV_16U ? 65535.0 : 255.0;
    double normalizedMedian = medianSample / inputRange;
    double MADN = MAGIC_1_4826 * medDev / inputRange;
    LOG_F(INFO, "Normalized median: {}, MADN: {}", normalizedMedian, MADN);

    bool upperHalf = normalizedMedian > 0.5;
    double shadows = 0.0;
    double highlights = 0.0;
    double midtones = 0.0;

    if (upperHalf || MADN == 0) {
        shadows = 0.0;
    } else {
        shadows =
            std::min(1.0, std::max(0.0, normalizedMedian - MAGIC_2_8 * MADN));
    }

    if (!upperHalf || MADN == 0) {
        highlights = 1.0;
    } else {
        highlights =
            std::min(1.0, std::max(0.0, normalizedMedian + MAGIC_2_8 * MADN));
    }

    double X = 0.0, M = 0.0;
    if (!upperHalf) {
        X = normalizedMedian - shadows;
        M = BASE_RATIO;
    } else {
        X = BASE_RATIO;
        M = highlights - normalizedMedian;
    }

    if (X == 0) {
        midtones = 0.0;
    } else if (X == M) {
        midtones = 0.5;
    } else if (X == 1) {
        midtones = 1.0;
    } else {
        midtones = ((M - 1) * X) / ((2 * M - 1) * X - M);
    }

    LOG_F(INFO, "Computed shadows: {}, midtones: {}, highlights: {}", shadows,
          midtones, highlights);
    return {shadows, midtones, highlights};
}

auto autoWhiteBalance(const cv::Mat& img) -> cv::Mat {
    LOG_F(INFO, "Starting auto white balance.");
    if (img.empty()) {
        LOG_F(ERROR, "Input image for autoWhiteBalance is empty.");
        THROW_INVALID_ARGUMENT("Input image for autoWhiteBalance is empty.");
    }
    if (img.channels() != 3) {
        LOG_F(ERROR, "Input image does not have 3 channels.");
        THROW_INVALID_ARGUMENT("Input image does not have 3 channels.");
    }

    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    double avgB = cv::mean(channels[0])[0];
    double avgG = cv::mean(channels[1])[0];
    double avgR = cv::mean(channels[2])[0];
    double avg = (avgB + avgG + avgR) / 3;

    LOG_F(INFO, "Averages - B: {}, G: {}, R: {}, Overall Avg: {}", avgB, avgG,
          avgR, avg);

    // Prevent division by zero
    double kb = (avgB != 0) ? avg / avgB : 1.0;
    double kg = (avgG != 0) ? avg / avgG : 1.0;
    double kr = (avgR != 0) ? avg / avgR : 1.0;

    channels[0] = channels[0] * kb;
    channels[1] = channels[1] * kg;
    channels[2] = channels[2] * kr;

    cv::Mat result;
    cv::merge(channels, result);
    LOG_F(INFO, "Completed auto white balance.");
    return result;
}
