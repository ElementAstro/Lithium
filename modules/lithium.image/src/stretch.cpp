
#include "stretch.hpp"
#include "imgutils.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <vector>

auto Stretch_WhiteBalance(const std::vector<cv::Mat>& hists,
                          const std::vector<cv::Mat>& bgr_planes) -> cv::Mat {
    std::vector<cv::Mat> planes;
    std::vector<double> highs;
    double maxPara = 0.0001;
    double minPara = 0.0001;

    for (size_t i = 0; i < hists.size(); ++i) {
        cv::Mat plane;
        std::vector<float> nonzeroIndices;
        cv::findNonZero(hists[i], nonzeroIndices);
        size_t nonezeroLen = nonzeroIndices.size();
        float minVal = nonzeroIndices[int(nonezeroLen * minPara)];
        float maxVal = nonzeroIndices[int(nonezeroLen * (1 - maxPara)) - 1];

        plane = bgr_planes[i].clone();
        plane.convertTo(plane, CV_32F);
        plane = (plane - minVal) / (maxVal - minVal) * 65535;
        cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);
        cv::threshold(plane, plane, 0, 0, cv::THRESH_TOZERO);
        plane.convertTo(plane, CV_16U);

        planes.push_back(plane);
        // TODO: Use cv::minMaxLoc
        // double high = (cv::minMaxLoc(hists[i]).maxVal - min_val) /
        //              (max_val - min_val) * 65535;
        double high = 0;
        highs.push_back(high);
    }

    double highMean =
        std::accumulate(highs.begin(), highs.end(), 0.0) / highs.size();

    std::vector<cv::Mat> adjustedPlanes;
    for (size_t i = 0; i < planes.size(); ++i) {
        cv::Mat temp;
        planes[i].convertTo(temp, CV_32F);
        temp *= (highMean / highs[i]);
        cv::threshold(temp, temp, 65535, 65535, cv::THRESH_TRUNC);
        temp.convertTo(temp, CV_16U);
        adjustedPlanes.push_back(temp);
    }

    cv::Mat dst;
    cv::merge(adjustedPlanes, dst);
    return dst;
}

auto StretchGray(const cv::Mat& hist, cv::Mat& plane) -> cv::Mat {
    double maxPara = 0.01;
    double minPara = 0.01;

    // Corrected: Using cv::Mat to store non-zero locations
    cv::Mat nonzeroLocations;
    cv::findNonZero(hist, nonzeroLocations);

    // Check if there are any nonzero elements
    if (nonzeroLocations.empty()) {
        return plane;  // or handle this condition appropriately
    }

    // Assuming hist is a histogram of intensities, we sort the locations to
    // find min and max If hist is not structured as expected, you'll need to
    // adjust this logic
    std::vector<int> nonzeroValues;
    nonzeroValues.reserve(nonzeroLocations.rows);
    for (int i = 0; i < nonzeroLocations.rows; ++i) {
        nonzeroValues.push_back(
            hist.at<float>(nonzeroLocations.at<cv::Point>(i).y, 0));
    }
    std::sort(nonzeroValues.begin(), nonzeroValues.end());

    size_t nonzeroLen = nonzeroValues.size();
    float minVal = nonzeroValues[int(nonzeroLen * minPara)];
    float maxVal = nonzeroValues[int(nonzeroLen * (1 - maxPara)) - 1];

    cv::threshold(plane, plane, maxVal, 65535, cv::THRESH_TRUNC);
    cv::threshold(plane, plane, minVal, 0, cv::THRESH_TOZERO);

    plane.convertTo(plane, CV_32F);
    plane = (plane - minVal) / (maxVal - minVal) * 65535;
    cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);

    // Corrected: Using cv::medianBlur correctly
    cv::Mat blurred;
    cv::medianBlur(plane, blurred,
                   3);  // Use a kernel size greater than 1 for actual blurring
    // Assuming you need to use the median of the blurred image for further
    // processing However, cv::medianBlur does not return a median value, so
    // you'll need a different approach to calculate median

    // Assuming a step here to calculate `gradMed` as it's not clear how you
    // intend to use medianBlur's result
    double gradMed = 5000;  // Placeholder value, calculate as needed

    double mt = gradMed / 30000;
    cv::pow(plane / 65535, 1 / mt, plane);
    plane *= 65535;
    cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);
    plane.convertTo(plane, CV_16U);

    return plane;
}

auto GrayStretch(const cv::Mat& img) -> cv::Mat {
    double blackClip = -1.25;
    double targetBkg = 0.1;
    cv::Mat normImg;
    cv::normalize(img, normImg, 0, 1, cv::NORM_MINMAX);

    // double median = cv::median(norm_img);
    double median = cv::mean(normImg).val[0];
    double avgbias = Cal_Avgdev(median, normImg);
    double c0 = std::min(std::max(median + (blackClip * avgbias), 0.0), 1.0);

    double m = (median - c0) * (targetBkg - 1) /
               ((((2 * targetBkg) - 1) * (median - c0)) - targetBkg);

    for (int i = 0; i < normImg.rows; i++) {
        for (int j = 0; j < normImg.cols; j++) {
            double value = normImg.at<double>(i, j);
            if (value < c0) {
                normImg.at<double>(i, j) = 0;
            } else {
                value = (value - c0) / (1 - c0);
                normImg.at<double>(i, j) = value;
            }
        }
    }

    normImg *= 65535;
    cv::Mat dstImg;
    normImg.convertTo(dstImg, CV_16U);
    return dstImg;
}

auto Stretch_OneChannel(const cv::Mat& norm_img, double shadows,
                        double midtones, double highlights) -> cv::Mat {
    cv::Mat result = norm_img.clone();
    double hsRangeFactor = 1.0;
    if (highlights != shadows) {
        hsRangeFactor = 1.0 / (highlights - shadows);
    }

    double k1 = (midtones - 1) * hsRangeFactor;
    double k2 = ((2 * midtones) - 1) * hsRangeFactor;

    double epsilon = 1e-10;

    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            double value = norm_img.at<double>(i, j);
            if (value < shadows) {
                result.at<double>(i, j) = 0;
            } else if (value > highlights) {
                result.at<double>(i, j) = 1;
            } else {
                result.at<double>(i, j) =
                    ((value - shadows) * k1 + epsilon) /
                    (((value - shadows) * k2) - midtones + epsilon);
            }
        }
    }
    return result;
}

auto stretchThreeChannels(const cv::Mat& img,
                          const std::vector<double>& shadows,
                          const std::vector<double>& midtones,
                          const std::vector<double>& highlights, int inputRange,
                          bool do_jpg) -> cv::Mat {
    cv::Mat dstImg = cv::Mat::zeros(img.size(), img.type());
    std::vector<cv::Mat> bgrPlanes;
    cv::split(img, bgrPlanes);
    cv::Mat& bPlane = bgrPlanes[0];
    cv::Mat& gPlane = bgrPlanes[1];
    cv::Mat& rPlane = bgrPlanes[2];

    int maxOutput = do_jpg ? 255 : 65535;
    double maxInput = inputRange - 1 > 0 ? inputRange - 1 : inputRange;

    double hsRangeFactorR =
        highlights[0] == shadows[0] ? 1.0 : 1.0 / (highlights[0] - shadows[0]);
    double hsRangeFactorG =
        highlights[1] == shadows[1] ? 1.0 : 1.0 / (highlights[1] - shadows[1]);
    double hsRangeFactorB =
        highlights[2] == shadows[2] ? 1.0 : 1.0 / (highlights[2] - shadows[2]);

    double nativeShadowsR = shadows[0] * maxInput;
    double nativeShadowsG = shadows[1] * maxInput;
    double nativeShadowsB = shadows[2] * maxInput;
    double nativeHighlightsR = highlights[0] * maxInput;
    double nativeHighlightsG = highlights[1] * maxInput;
    double nativeHighlightsB = highlights[2] * maxInput;

    double k1R = (midtones[0] - 1) * hsRangeFactorR * maxOutput / maxInput;
    double k1G = (midtones[1] - 1) * hsRangeFactorG * maxOutput / maxInput;
    double k1B = (midtones[2] - 1) * hsRangeFactorB * maxOutput / maxInput;

    double k2R = ((2 * midtones[0]) - 1) * hsRangeFactorR / maxInput;
    double k2G = ((2 * midtones[1]) - 1) * hsRangeFactorG / maxInput;
    double k2B = ((2 * midtones[2]) - 1) * hsRangeFactorB / maxInput;

    double epsilon = 1e-10;

    for (int i = 0; i < bPlane.rows; i++) {
        for (int j = 0; j < bPlane.cols; j++) {
            // Process B channel
            double bValue = bPlane.at<uchar>(i, j);
            if (bValue < nativeShadowsB) {
                bPlane.at<uchar>(i, j) = 0;
            } else if (bValue > nativeHighlightsB) {
                bPlane.at<uchar>(i, j) = maxOutput;
            } else {
                bPlane.at<uchar>(i, j) = static_cast<uchar>((
                    ((bValue - nativeShadowsB) * k1B + epsilon) /
                    ((bValue - nativeShadowsB) * k2B - midtones[2] + epsilon)));
            }

            // Process G channel
            double gValue = gPlane.at<uchar>(i, j);
            if (gValue < nativeShadowsG) {
                gPlane.at<uchar>(i, j) = 0;
            } else if (gValue > nativeHighlightsG) {
                gPlane.at<uchar>(i, j) = maxOutput;
            } else {
                gPlane.at<uchar>(i, j) = static_cast<uchar>((
                    ((gValue - nativeShadowsG) * k1G + epsilon) /
                    ((gValue - nativeShadowsG) * k2G - midtones[1] + epsilon)));
            }

            // Process R channel
            double rValue = rPlane.at<uchar>(i, j);
            if (rValue < nativeShadowsR) {
                rPlane.at<uchar>(i, j) = 0;
            } else if (rValue > nativeHighlightsR) {
                rPlane.at<uchar>(i, j) = maxOutput;
            } else {
                rPlane.at<uchar>(i, j) = static_cast<uchar>((
                    ((rValue - nativeShadowsR) * k1R + epsilon) /
                    ((rValue - nativeShadowsR) * k2R - midtones[0] + epsilon)));
            }
        }
    }

    cv::merge(bgrPlanes, dstImg);
    return dstImg;
}
