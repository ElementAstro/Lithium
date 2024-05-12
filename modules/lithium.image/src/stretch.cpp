
#include "stretch.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <vector>


cv::Mat Stretch_WhiteBalance(const std::vector<cv::Mat>& hists,
                             const std::vector<cv::Mat>& bgr_planes) {
    std::vector<cv::Mat> planes;
    std::vector<double> highs;
    double max_para = 0.0001;
    double min_para = 0.0001;

    for (size_t i = 0; i < hists.size(); ++i) {
        cv::Mat plane;
        std::vector<float> nonzero_indices;
        cv::findNonZero(hists[i], nonzero_indices);
        size_t nonezero_len = nonzero_indices.size();
        float min_val = nonzero_indices[int(nonezero_len * min_para)];
        float max_val = nonzero_indices[int(nonezero_len * (1 - max_para)) - 1];

        plane = bgr_planes[i].clone();
        plane.convertTo(plane, CV_32F);
        plane = (plane - min_val) / (max_val - min_val) * 65535;
        cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);
        cv::threshold(plane, plane, 0, 0, cv::THRESH_TOZERO);
        plane.convertTo(plane, CV_16U);

        planes.push_back(plane);
        double high = (cv::minMaxLoc(hists[i]).maxVal - min_val) /
                      (max_val - min_val) * 65535;
        highs.push_back(high);
    }

    double high_mean =
        std::accumulate(highs.begin(), highs.end(), 0.0) / highs.size();

    std::vector<cv::Mat> adjusted_planes;
    for (size_t i = 0; i < planes.size(); ++i) {
        cv::Mat temp;
        planes[i].convertTo(temp, CV_32F);
        temp *= (high_mean / highs[i]);
        cv::threshold(temp, temp, 65535, 65535, cv::THRESH_TRUNC);
        temp.convertTo(temp, CV_16U);
        adjusted_planes.push_back(temp);
    }

    cv::Mat dst;
    cv::merge(adjusted_planes, dst);
    return dst;
}

cv::Mat StretchGray(const cv::Mat& hist, cv::Mat& plane) {
    double max_para = 0.01;
    double min_para = 0.01;

    // Corrected: Using cv::Mat to store non-zero locations
    cv::Mat nonzero_locations;
    cv::findNonZero(hist, nonzero_locations);

    // Check if there are any nonzero elements
    if (nonzero_locations.empty()) {
        return plane;  // or handle this condition appropriately
    }

    // Assuming hist is a histogram of intensities, we sort the locations to
    // find min and max If hist is not structured as expected, you'll need to
    // adjust this logic
    std::vector<int> nonzero_values;
    for (int i = 0; i < nonzero_locations.rows; ++i) {
        nonzero_values.push_back(
            hist.at<float>(nonzero_locations.at<cv::Point>(i).y, 0));
    }
    std::sort(nonzero_values.begin(), nonzero_values.end());

    size_t nonzero_len = nonzero_values.size();
    float min_val = nonzero_values[int(nonzero_len * min_para)];
    float max_val = nonzero_values[int(nonzero_len * (1 - max_para)) - 1];

    cv::threshold(plane, plane, max_val, 65535, cv::THRESH_TRUNC);
    cv::threshold(plane, plane, min_val, 0, cv::THRESH_TOZERO);

    plane.convertTo(plane, CV_32F);
    plane = (plane - min_val) / (max_val - min_val) * 65535;
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

    double Mt = gradMed / 30000;
    cv::pow(plane / 65535, 1 / Mt, plane);
    plane *= 65535;
    cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);
    plane.convertTo(plane, CV_16U);

    return plane;
}

cv::Mat GrayStretch(const cv::Mat& img) {
    double black_clip = -1.25;
    double target_bkg = 0.1;
    cv::Mat norm_img;
    cv::normalize(img, norm_img, 0, 1, cv::NORM_MINMAX);

    // double median = cv::median(norm_img);
    double median = cv::mean(norm_img);
    double avgbias = Cal_Avgdev(median, norm_img);
    double c0 = std::min(std::max(median + (black_clip * avgbias), 0.0), 1.0);

    double m = (median - c0) * (target_bkg - 1) /
               ((((2 * target_bkg) - 1) * (median - c0)) - target_bkg);

    for (int i = 0; i < norm_img.rows; i++) {
        for (int j = 0; j < norm_img.cols; j++) {
            double value = norm_img.at<double>(i, j);
            if (value < c0) {
                norm_img.at<double>(i, j) = 0;
            } else {
                value = (value - c0) / (1 - c0);
                norm_img.at<double>(i, j) = MTF(m, value);
            }
        }
    }

    norm_img *= 65535;
    cv::Mat dst_img;
    norm_img.convertTo(dst_img, CV_16U);
    return dst_img;
}

cv::Mat Stretch_OneChannel(const cv::Mat& norm_img, double shadows,
                           double midtones, double highlights) {
    cv::Mat result = norm_img.clone();
    double hsRangeFactor = 1.0;
    if (highlights != shadows)
        hsRangeFactor = 1.0 / (highlights - shadows);

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

cv::Mat stretchThreeChannels(const cv::Mat& img,
                             const std::vector<double>& shadows,
                             const std::vector<double>& midtones,
                             const std::vector<double>& highlights,
                             int inputRange, bool do_jpg) {
    cv::Mat dst_img = cv::Mat::zeros(img.size(), img.type());
    std::vector<cv::Mat> bgr_planes;
    cv::split(img, bgr_planes);
    cv::Mat& b_plane = bgr_planes[0];
    cv::Mat& g_plane = bgr_planes[1];
    cv::Mat& r_plane = bgr_planes[2];

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

    for (int i = 0; i < b_plane.rows; i++) {
        for (int j = 0; j < b_plane.cols; j++) {
            // Process B channel
            double b_value = b_plane.at<uchar>(i, j);
            if (b_value < nativeShadowsB) {
                b_plane.at<uchar>(i, j) = 0;
            } else if (b_value > nativeHighlightsB) {
                b_plane.at<uchar>(i, j) = maxOutput;
            } else {
                b_plane.at<uchar>(i, j) = static_cast<uchar>(
                    (((b_value - nativeShadowsB) * k1B + epsilon) /
                     ((b_value - nativeShadowsB) * k2B - midtones[2] +
                      epsilon)));
            }

            // Process G channel
            double g_value = g_plane.at<uchar>(i, j);
            if (g_value < nativeShadowsG) {
                g_plane.at<uchar>(i, j) = 0;
            } else if (g_value > nativeHighlightsG) {
                g_plane.at<uchar>(i, j) = maxOutput;
            } else {
                g_plane.at<uchar>(i, j) = static_cast<uchar>(
                    (((g_value - nativeShadowsG) * k1G + epsilon) /
                     ((g_value - nativeShadowsG) * k2G - midtones[1] +
                      epsilon)));
            }

            // Process R channel
            double r_value = r_plane.at<uchar>(i, j);
            if (r_value < nativeShadowsR) {
                r_plane.at<uchar>(i, j) = 0;
            } else if (r_value > nativeHighlightsR) {
                r_plane.at<uchar>(i, j) = maxOutput;
            } else {
                r_plane.at<uchar>(i, j) = static_cast<uchar>(
                    (((r_value - nativeShadowsR) * k1R + epsilon) /
                     ((r_value - nativeShadowsR) * k2R - midtones[0] +
                      epsilon)));
            }
        }
    }

    cv::merge(bgr_planes, dst_img);
    return dst_img;
}