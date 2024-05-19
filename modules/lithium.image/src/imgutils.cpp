#include "imgutils.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "atom/log/loguru.hpp"

cv::Mat loadImage(const std::string& filename, bool isGrayscale = false) {
    int flags = isGrayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR;
    cv::Mat image = cv::imread(filename, flags);
    if (image.empty()) {
        LOG_F(ERROR, "Failed to load image: {}", filename);
        return cv::Mat();
    }
    return image;
}

// 从文件夹中读取所有图像
std::vector<cv::Mat> loadImages(const std::string& folder,
                                const std::vector<std::string>& filenames) {
    std::vector<cv::Mat> images;
    for (const auto& filename : filenames) {
        cv::Mat img = cv::imread(folder + "/" + filename, cv::IMREAD_COLOR);
        if (!img.empty()) {
            images.push_back(img);
        } else {
            LOG_F(ERROR, "Failed to load image: {}", filename);
        }
    }
    return images;
}

bool insideCircle(int x, int y, int centerX, int centerY, float radius) {
    return std::sqrt((x - centerX) * (x - centerX) +
                     (y - centerY) * (y - centerY)) < radius;
}

bool checkElongated(int width, int height) {
    double minlongratio = 1.5;
    double ratio = width > height ? static_cast<double>(width) / height
                                  : static_cast<double>(height) / width;
    return ratio > minlongratio;
}

int checkWhitePixel(const cv::Mat& rect_contour, int x, int y) {
    if (x >= 0 && x < rect_contour.cols && y >= 0 && y < rect_contour.rows) {
        return rect_contour.at<uint16_t>(y, x) > 0 ? 1 : 0;
    }
    return 0;
}

int EightSymmetryCircleCheck(const cv::Mat& rect_contour,
                             const cv::Point& center, int x_p, int y_p) {
    int whitepixel = 0;
    whitepixel += checkWhitePixel(rect_contour, center.x + x_p, center.y + y_p);
    whitepixel += checkWhitePixel(rect_contour, center.x - x_p, center.y + y_p);
    whitepixel += checkWhitePixel(rect_contour, center.x + x_p, center.y - y_p);
    whitepixel += checkWhitePixel(rect_contour, center.x - x_p, center.y - y_p);
    whitepixel += checkWhitePixel(rect_contour, center.x + y_p, center.y + x_p);
    whitepixel += checkWhitePixel(rect_contour, center.x + y_p, center.y - x_p);
    whitepixel += checkWhitePixel(rect_contour, center.x - y_p, center.y + x_p);
    whitepixel += checkWhitePixel(rect_contour, center.x - y_p, center.y - x_p);
    return whitepixel;
}

int FourSymmetryCircleCheck(const cv::Mat& rect_contour,
                            const cv::Point& center, float radius) {
    int whitepixel = 0;
    whitepixel += checkWhitePixel(rect_contour, center.x,
                                  center.y + static_cast<int>(radius));
    whitepixel += checkWhitePixel(rect_contour, center.x,
                                  center.y - static_cast<int>(radius));
    whitepixel += checkWhitePixel(
        rect_contour, center.x - static_cast<int>(radius), center.y);
    whitepixel += checkWhitePixel(
        rect_contour, center.x + static_cast<int>(radius), center.y);
    return whitepixel;
}

std::tuple<int, std::vector<int>, std::vector<double>> define_narrow_radius(
    int min_area, double max_area, double area, double scale) {
    std::vector<int> checklist;
    std::vector<double> threslist;
    int checknum = 0;

    if (min_area <= area && area <= 500 * scale) {
        checknum = 2;
        checklist = {1, 2};
        threslist = {0.5, 0.65};
    } else if (500 * scale < area && area <= 1000 * scale) {
        checknum = 3;
        checklist = {2, 3, 4};
        threslist = {0.5, 0.65, 0.75};
    } else if (1000 * scale < area && area <= max_area) {
        checknum = 3;
        checklist = {2, 3, 4};
        threslist = {0.5, 0.65, 0.75};
    } else {
        checknum = 0;
        checklist = {};
        threslist = {};
    }
    return {checknum, checklist, threslist};
}

bool BresenHamCheckCircle(const cv::Mat& rect_contour, float radius,
                          float pixelratio, bool if_debug) {
    cv::Mat rect_contour_rgb;
    if (if_debug) {
        cv::cvtColor(rect_contour, rect_contour_rgb, cv::COLOR_GRAY2BGR);
    }

    int totalpixel = 0;
    int whitepixel = 0;

    cv::Size shps = rect_contour.size();
    cv::Point center(shps.width / 2, shps.height / 2);

    int p = 1 - static_cast<int>(radius);
    int x_p = 0;
    int y_p = static_cast<int>(radius);
    whitepixel += FourSymmetryCircleCheck(rect_contour, center, radius);
    totalpixel += 4;

    while (x_p <= y_p) {
        x_p += 1;
        if (p < 0) {
            p += 2 * x_p + 1;
        } else {
            y_p -= 1;
            p += 2 * (x_p - y_p) + 1;
        }

        if (if_debug) {
            int singlewhite;
            // std::tie(singlewhite, rect_contour_rgb) =
            // EightSymmetryCircleCheck_forDebug(rect_contour, rect_contour_rgb,
            // center, x_p, y_p);
            whitepixel += singlewhite;
        } else {
            whitepixel +=
                EightSymmetryCircleCheck(rect_contour, center, x_p, y_p);
        }

        totalpixel += 8;
    }

    float ratio = static_cast<float>(whitepixel) / totalpixel;
    if (if_debug) {
        std::cout << "ratio: " << ratio << std::endl;
    }

    return ratio > pixelratio;
}

double Cal_Avgdev(double mid, const cv::Mat& norm_img) {
    int size = norm_img.rows * norm_img.cols;
    double sum = 0;
    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            sum += std::abs(norm_img.at<double>(i, j) - mid);
        }
    }
    double avg_dev = sum / size;
    return avg_dev;
}

cv::Mat MTF(double m, const cv::Mat& img) {
    cv::Mat result = img.clone();
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            double value = img.at<double>(i, j);
            if (value != 0 && value != m && value != 1) {
                result.at<double>(i, j) =
                    (m - 1) * value / (((2 * m - 1) * value) - m);
            }
        }
    }
    return result;
}

double CalScale(const cv::Mat& img, int resize_size) {
    double scale = (img.rows > img.cols) ? (double)resize_size / img.rows
                                         : (double)resize_size / img.cols;
    return scale;
}

double Cal_Middev(double mid, const cv::Mat& img) {
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

std::tuple<double, double, double> computeParamsOneChannel(const cv::Mat& img) {
    // Flatten the image and sample it
    std::vector<uchar> buffer_value;
    if (img.isContinuous()) {
        buffer_value.assign(img.data, img.data + img.total());
    } else {
        for (int i = 0; i < img.rows; ++i) {
            buffer_value.insert(buffer_value.end(), img.ptr<uchar>(i),
                                img.ptr<uchar>(i) + img.cols);
        }
    }

    int maxSamples = 500000;
    int sampleBy = (img.rows * img.cols < maxSamples)
                       ? 1
                       : (img.rows * img.cols / maxSamples);

    std::vector<uchar> sample_value;
    for (size_t i = 0; i < buffer_value.size(); i += sampleBy) {
        sample_value.push_back(buffer_value[i]);
    }

    // Compute median of sampled values
    size_t n = sample_value.size() / 2;
    std::nth_element(sample_value.begin(), sample_value.begin() + n,
                     sample_value.end());
    double medianSample = sample_value[n];

    // Compute Median Absolute Deviation (MAD)
    std::vector<double> abs_dev(sample_value.size());
    std::transform(
        sample_value.begin(), sample_value.end(), abs_dev.begin(),
        [medianSample](uchar v) { return std::abs(v - medianSample); });

    std::nth_element(abs_dev.begin(), abs_dev.begin() + n, abs_dev.end());
    double medDev = abs_dev[n];

    // Normalize
    double inputRange = img.depth() == CV_16U ? 65535.0 : 255.0;
    double normalizedMedian = medianSample / inputRange;
    double MADN = 1.4826 * medDev / inputRange;

    const double B = 0.25;
    bool upper_half = normalizedMedian > 0.5;
    double shadows, highlights, midtones;

    if (upper_half || MADN == 0) {
        shadows = 0.0;
    } else {
        shadows = std::min(1.0, std::max(0.0, normalizedMedian + -2.8 * MADN));
    }

    if (!upper_half || MADN == 0) {
        highlights = 1.0;
    } else {
        highlights =
            std::min(1.0, std::max(0.0, normalizedMedian - -2.8 * MADN));
    }

    double X, M;
    if (!upper_half) {
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

cv::Mat Auto_WhiteBalance(const cv::Mat& img) {
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

void bilateralFilter(const cv::Mat& src, cv::Mat& dst, int d, double sigmaColor,
                     double sigmaSpace) {
    cv::bilateralFilter(src, dst, d, sigmaColor, sigmaSpace);
}

void sharpen(const cv::Mat& src, cv::Mat& dst) {
    // A simple sharpening kernel
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);

    cv::filter2D(src, dst, -1, kernel);
}
