#include "imgutils.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "atom/log/loguru.hpp"

constexpr double MIN_LONG_RATIO = 1.5;
constexpr int MAX_SAMPLES = 500000;
constexpr double MAGIC_1_4826 = 1.4826;
constexpr double MAGIC_2_8 = 2.8;
constexpr double B = 0.25;
constexpr int KERNEL_SIZE = 3;
constexpr int SHARPEN_VALUE = 5;

auto insideCircle(int xCoord, int yCoord, int centerX, int centerY,
                  float radius) -> bool {
    return std::sqrt((xCoord - centerX) * (xCoord - centerX) +
                     (yCoord - centerY) * (yCoord - centerY)) < radius;
}

auto checkElongated(int width, int height) -> bool {
    double ratio = width > height ? static_cast<double>(width) / height
                                  : static_cast<double>(height) / width;
    return ratio > MIN_LONG_RATIO;
}

auto checkWhitePixel(const cv::Mat& rect_contour, int xCoord,
                     int yCoord) -> int {
    if (xCoord >= 0 && xCoord < rect_contour.cols && yCoord >= 0 &&
        yCoord < rect_contour.rows) {
        return rect_contour.at<uint16_t>(yCoord, xCoord) > 0 ? 1 : 0;
    }
    return 0;
}

auto EightSymmetryCircleCheck(const cv::Mat& rect_contour,
                              const cv::Point& center, int xCoord,
                              int yCoord) -> int {
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
    return whitePixelCount;
}

auto FourSymmetryCircleCheck(const cv::Mat& rect_contour,
                             const cv::Point& center, float radius) -> int {
    int whitePixelCount = 0;
    whitePixelCount += checkWhitePixel(rect_contour, center.x,
                                       center.y + static_cast<int>(radius));
    whitePixelCount += checkWhitePixel(rect_contour, center.x,
                                       center.y - static_cast<int>(radius));
    whitePixelCount += checkWhitePixel(
        rect_contour, center.x - static_cast<int>(radius), center.y);
    whitePixelCount += checkWhitePixel(
        rect_contour, center.x + static_cast<int>(radius), center.y);
    return whitePixelCount;
}

auto define_narrow_radius(int min_area, double max_area, double area,
                          double scale)
    -> std::tuple<int, std::vector<int>, std::vector<double>> {
    std::vector<int> checklist;
    std::vector<double> thresholdList;
    int checkNum = 0;

    if (min_area <= area && area <= 500 * scale) {
        checkNum = 2;
        checklist = {1, 2};
        thresholdList = {0.5, 0.65};
    } else if (500 * scale < area && area <= 1000 * scale) {
        checkNum = 3;
        checklist = {2, 3, 4};
        thresholdList = {0.5, 0.65, 0.75};
    } else if (1000 * scale < area && area <= max_area) {
        checkNum = 3;
        checklist = {2, 3, 4};
        thresholdList = {0.5, 0.65, 0.75};
    } else {
        checkNum = 0;
        checklist = {};
        thresholdList = {};
    }
    return {checkNum, checklist, thresholdList};
}

auto BresenHamCheckCircle(const cv::Mat& rect_contour, float radius,
                          float pixelRatio, bool if_debug) -> bool {
    cv::Mat rect_contour_rgb;
    if (if_debug) {
        cv::cvtColor(rect_contour, rect_contour_rgb, cv::COLOR_GRAY2BGR);
    }

    int totalPixelCount = 0;
    int whitePixelCount = 0;

    cv::Size shape = rect_contour.size();
    cv::Point center(shape.width / 2, shape.height / 2);

    int p = 1 - static_cast<int>(radius);
    int xCoord = 0;
    int yCoord = static_cast<int>(radius);
    whitePixelCount += FourSymmetryCircleCheck(rect_contour, center, radius);
    totalPixelCount += 4;

    while (xCoord <= yCoord) {
        xCoord += 1;
        if (p < 0) {
            p += 2 * xCoord + 1;
        } else {
            yCoord -= 1;
            p += 2 * (xCoord - yCoord) + 1;
        }

        if (if_debug) {
            int singleWhitePixelCount = 0;
            // std::tie(singleWhitePixelCount, rect_contour_rgb) =
            // EightSymmetryCircleCheck_forDebug(rect_contour, rect_contour_rgb,
            // center, xCoord, yCoord);
            whitePixelCount += singleWhitePixelCount;
        } else {
            whitePixelCount +=
                EightSymmetryCircleCheck(rect_contour, center, xCoord, yCoord);
        }

        totalPixelCount += 8;
    }

    float ratio = static_cast<float>(whitePixelCount) / totalPixelCount;
    if (if_debug) {
        std::cout << "ratio: " << ratio << std::endl;
    }

    return ratio > pixelRatio;
}

auto Cal_Avgdev(double mid, const cv::Mat& norm_img) -> double {
    int size = norm_img.rows * norm_img.cols;
    double sum = 0;
    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            sum += std::abs(norm_img.at<double>(i, j) - mid);
        }
    }
    double avgDev = sum / size;
    return avgDev;
}

auto MTF(double mean, const cv::Mat& img) -> cv::Mat {
    cv::Mat result = img.clone();
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            double value = img.at<double>(i, j);
            if (value != 0 && value != mean && value != 1) {
                result.at<double>(i, j) =
                    (mean - 1) * value / (((2 * mean - 1) * value) - mean);
            }
        }
    }
    return result;
}

auto CalScale(const cv::Mat& img, int resize_size) -> double {
    double scale = (img.rows > img.cols)
                       ? static_cast<double>(resize_size) / img.rows
                       : static_cast<double>(resize_size) / img.cols;
    return scale;
}

auto Cal_Middev(double mid, const cv::Mat& img) -> double {
    std::vector<double> deviations;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            deviations.push_back(std::abs(img.at<double>(i, j) - mid));
        }
    }
    std::nth_element(deviations.begin(),
                     deviations.begin() + deviations.size() / 2,
                     deviations.end());
    return deviations[deviations.size() / 2];
}

auto computeParamsOneChannel(const cv::Mat& img)
    -> std::tuple<double, double, double> {
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

    // Compute median of sampled values
    size_t n = sampleValue.size() / 2;
    std::nth_element(sampleValue.begin(), sampleValue.begin() + n,
                     sampleValue.end());
    double medianSample = sampleValue[n];

    // Compute Median Absolute Deviation (MAD)
    std::vector<double> absDev(sampleValue.size());
    std::transform(
        sampleValue.begin(), sampleValue.end(), absDev.begin(),
        [medianSample](uchar value) { return std::abs(value - medianSample); });

    std::nth_element(absDev.begin(), absDev.begin() + n, absDev.end());
    double medDev = absDev[n];

    // Normalize
    double inputRange = img.depth() == CV_16U ? 65535.0 : 255.0;
    double normalizedMedian = medianSample / inputRange;
    double MADN = MAGIC_1_4826 * medDev / inputRange;

    bool upperHalf = normalizedMedian > 0.5;
    double shadows, highlights, midtones;

    if (upperHalf || MADN == 0) {
        shadows = 0.0;
    } else {
        shadows =
            std::min(1.0, std::max(0.0, normalizedMedian + -MAGIC_2_8 * MADN));
    }

    if (!upperHalf || MADN == 0) {
        highlights = 1.0;
    } else {
        highlights =
            std::min(1.0, std::max(0.0, normalizedMedian - -MAGIC_2_8 * MADN));
    }

    double X, M;
    if (!upperHalf) {
        X = normalizedMedian - shadows;
        M = B;
    } else {
        X = B;
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

    return {shadows, midtones, highlights};
}

auto autoWhiteBalance(const cv::Mat& img) -> cv::Mat {
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    double avgB = cv::mean(channels[0])[0];
    double avgG = cv::mean(channels[1])[0];
    double avgR = cv::mean(channels[2])[0];
    double avg = (avgB + avgG + avgR) / 3;

    double kb = avg / avgB;
    double kg = avg / avgG;
    double kr = avg / avgR;

    channels[0] = channels[0] * kb;
    channels[1] = channels[1] * kg;
    channels[2] = channels[2] * kr;

    cv::Mat result;
    cv::merge(channels, result);
    return result;
}

void averageFilter(const cv::Mat& src, cv::Mat& dst, int kernelSize) {
    cv::blur(src, dst, cv::Size(kernelSize, kernelSize));
}

void gaussianFilter(const cv::Mat& src, cv::Mat& dst, int kernelSize,
                    double sigmaX, double sigmaY = 0) {
    cv::GaussianBlur(src, dst, cv::Size(kernelSize, kernelSize), sigmaX,
                     sigmaY);
}

void medianFilter(const cv::Mat& src, cv::Mat& dst, int kernelSize) {
    cv::medianBlur(src, dst, kernelSize);
}

void bilateralFilter(const cv::Mat& src, cv::Mat& dst, int diameter,
                     double sigmaColor, double sigmaSpace) {
    cv::bilateralFilter(src, dst, diameter, sigmaColor, sigmaSpace);
}

void sharpen(const cv::Mat& src, cv::Mat& dst) {
    // A simple sharpening kernel
    cv::Mat kernel = (cv::Mat_<float>(KERNEL_SIZE, KERNEL_SIZE) << 0, -1, 0, -1,
                      SHARPEN_VALUE, -1, 0, -1, 0);

    cv::filter2D(src, dst, -1, kernel);
}