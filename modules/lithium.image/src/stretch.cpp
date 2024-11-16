
#include "stretch.hpp"
#include "imgutils.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

constexpr double BLACK_CLIP_FACTOR = -1.25;
constexpr double TARGET_BACKGROUND = 0.1;
constexpr int MAX_8BIT_VALUE = 255;
constexpr int MAX_16BIT_VALUE = 65535;

auto stretchWhiteBalance(const std::vector<cv::Mat>& hists,
                         const std::vector<cv::Mat>& bgrPlanes) -> cv::Mat {
    LOG_F(INFO,
          "Starting white balance stretch for image {}x{} with {} channels",
          bgrPlanes[0].cols, bgrPlanes[0].rows, bgrPlanes.size());

    if (hists.size() != 3 || bgrPlanes.size() != 3) {
        LOG_F(ERROR, "Invalid input dimensions: hists={}, planes={}",
              hists.size(), bgrPlanes.size());
        THROW_INVALID_ARGUMENT(
            "Both hists and bgrPlanes must contain 3 channels");
    }

    std::vector<cv::Mat> planes;
    std::vector<double> highs;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < hists.size(); ++i) {
        LOG_F(INFO, "Processing channel {} of {}", i + 1, hists.size());

        cv::Mat plane;
        cv::Mat nonzeroLocations;
        cv::findNonZero(hists[i], nonzeroLocations);

        if (nonzeroLocations.empty()) {
            LOG_F(WARNING, "Channel {} has no non-zero values, skipping", i);
            continue;
        }

        size_t nonzeroLen = nonzeroLocations.total();
        LOG_F(INFO, "Channel {} has {} non-zero values", i, nonzeroLen);

        float minVal =
            nonzeroLocations.at<cv::Point>(int(nonzeroLen * DEFAULT_MIN_PARA))
                .y;
        float maxVal =
            nonzeroLocations
                .at<cv::Point>(int(nonzeroLen * (1 - DEFAULT_MAX_PARA)) - 1)
                .y;

        LOG_F(INFO, "Channel {} value range: min={:.2f}, max={:.2f}", i, minVal,
              maxVal);

        plane = bgrPlanes[i].clone();
        plane.convertTo(plane, CV_32F);

        LOG_F(INFO, "Stretching channel {} values to 16-bit range", i);
        plane = (plane - minVal) / (maxVal - minVal) * 65535;

        LOG_F(INFO, "Applying thresholds to channel {}", i);
        cv::threshold(plane, plane, 65535, 65535, cv::THRESH_TRUNC);
        cv::threshold(plane, plane, 0, 0, cv::THRESH_TOZERO);
        plane.convertTo(plane, CV_16U);

        double minHist, maxHist;
        cv::minMaxLoc(hists[i], &minHist, &maxHist);
        double high = (maxHist - minVal) / (maxVal - minVal) * 65535;

        planes.push_back(plane);
        highs.push_back(high);

        LOG_F(INFO,
              "Channel {} processing complete: min={:.2f}, max={:.2f}, "
              "high={:.2f}",
              i, minVal, maxVal, high);
    }

    double highMean =
        std::accumulate(highs.begin(), highs.end(), 0.0) / highs.size();
    LOG_F(INFO, "Calculated average high value: {:.2f}", highMean);

    LOG_F(INFO, "Adjusting channel intensities...");
    std::vector<cv::Mat> adjustedPlanes;
    for (size_t i = 0; i < planes.size(); ++i) {
        LOG_F(INFO, "Adjusting channel {} with factor {:.3f}", i,
              highMean / highs[i]);

        cv::Mat temp;
        planes[i].convertTo(temp, CV_32F);
        temp *= (highMean / highs[i]);
        cv::threshold(temp, temp, 65535, 65535, cv::THRESH_TRUNC);
        temp.convertTo(temp, CV_16U);
        adjustedPlanes.push_back(temp);
    }

    cv::Mat dst;
    cv::merge(adjustedPlanes, dst);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    LOG_F(INFO, "White balance stretch completed in {} ms. Output size: {}x{}",
          duration.count(), dst.cols, dst.rows);

    return dst;
}

auto stretchGray(const cv::Mat& hist, cv::Mat& plane) -> cv::Mat {
    LOG_F(INFO, "Starting grayscale stretch");

    if (hist.empty() || plane.empty()) {
        LOG_F(ERROR, "Empty input histogram or plane");
        THROW_INVALID_ARGUMENT("Input histogram or plane is empty");
    }

    cv::Mat nonzeroLocations;
    cv::findNonZero(hist, nonzeroLocations);

    if (nonzeroLocations.empty()) {
        LOG_F(WARNING, "No non-zero values found in histogram");
        return plane;
    }

    std::vector<int> nonzeroValues;
    nonzeroValues.reserve(nonzeroLocations.rows);
    for (int i = 0; i < nonzeroLocations.rows; ++i) {
        nonzeroValues.push_back(
            hist.at<float>(nonzeroLocations.at<cv::Point>(i).y, 0));
    }
    std::sort(nonzeroValues.begin(), nonzeroValues.end());

    size_t nonzeroLen = nonzeroValues.size();
    float minVal = nonzeroValues[int(nonzeroLen * DEFAULT_MIN_PARA)];
    float maxVal = nonzeroValues[int(nonzeroLen * (1 - DEFAULT_MAX_PARA)) - 1];

    LOG_F(INFO, "Calculated stretch parameters: min={}, max={}", minVal,
          maxVal);

    cv::Mat stretched;
    plane.convertTo(stretched, CV_32F);
    stretched = (stretched - minVal) / (maxVal - minVal) * 65535;

    cv::threshold(stretched, stretched, 65535, 65535, cv::THRESH_TRUNC);
    cv::threshold(stretched, stretched, 0, 0, cv::THRESH_TOZERO);

    cv::Mat medianBlurred;
    cv::medianBlur(stretched, medianBlurred, DEFAULT_MEDIAN_BLUR_SIZE);

    double medianValue;
    cv::Scalar mean = cv::mean(medianBlurred);
    medianValue = mean[0];
    double gradMed = medianValue;

    double mt = gradMed / 30000;
    cv::pow(stretched / 65535, 1 / mt, stretched);
    stretched *= 65535;

    cv::threshold(stretched, stretched, 65535, 65535, cv::THRESH_TRUNC);
    stretched.convertTo(stretched, CV_16U);

    LOG_F(INFO, "Grayscale stretch completed");
    return stretched;
}

auto calculateAverageDeviation(double median,
                               const cv::Mat& normalizedImg) -> double {
    double sum = 0.0;
    for (int i = 0; i < normalizedImg.rows; i++) {
        for (int j = 0; j < normalizedImg.cols; j++) {
            sum += std::abs(normalizedImg.at<double>(i, j) - median);
        }
    }
    return sum / (normalizedImg.rows * normalizedImg.cols);
}

auto grayStretch(const cv::Mat& img) -> cv::Mat {
    cv::Mat normalizedImg;
    cv::normalize(img, normalizedImg, 0, 1, cv::NORM_MINMAX);

    double median = cv::mean(normalizedImg).val[0];
    double averageDeviation = calculateAverageDeviation(median, normalizedImg);
    double clipLevel = std::min(
        std::max(median + (BLACK_CLIP_FACTOR * averageDeviation), 0.0), 1.0);

    double multiplier =
        (median - clipLevel) * (TARGET_BACKGROUND - 1) /
        ((((2 * TARGET_BACKGROUND) - 1) * (median - clipLevel)) -
         TARGET_BACKGROUND);

    for (int i = 0; i < normalizedImg.rows; i++) {
        for (int j = 0; j < normalizedImg.cols; j++) {
            double value = normalizedImg.at<double>(i, j);
            if (value < clipLevel) {
                normalizedImg.at<double>(i, j) = 0;
            } else {
                value = (value - clipLevel) / (1 - clipLevel);
                normalizedImg.at<double>(i, j) = value;
            }
        }
    }

    normalizedImg *= MAX_16BIT_VALUE;
    cv::Mat destImg;
    normalizedImg.convertTo(destImg, CV_16U);
    return destImg;
}

auto stretchOneChannel(const cv::Mat& normalizedImg,
                       const StretchParams& params) -> cv::Mat {
    cv::Mat result = normalizedImg.clone();
    double rangeScale = (params.highlights != params.shadows)
                            ? 1.0 / (params.highlights - params.shadows)
                            : 1.0;

    double factorK1 = (params.tones - 1) * rangeScale;
    double factorK2 = ((2 * params.tones) - 1) * rangeScale;

    for (int i = 0; i < normalizedImg.rows; i++) {
        for (int j = 0; j < normalizedImg.cols; j++) {
            double value = normalizedImg.at<double>(i, j);
            if (value < params.shadows) {
                result.at<double>(i, j) = 0;
            } else if (value > params.highlights) {
                result.at<double>(i, j) = 1;
            } else {
                result.at<double>(i, j) =
                    ((value - params.shadows) * factorK1 + EPSILON) /
                    (((value - params.shadows) * factorK2) - params.tones +
                     EPSILON);
            }
        }
    }
    return result;
}

auto stretchThreeChannels(const cv::Mat& img,
                          const std::vector<StretchParams>& channelParams,
                          int inputRange, bool useJpeg) -> cv::Mat {
    LOG_F(INFO,
          "Starting three channel stretch: size={}x{}, input_range={}, "
          "jpeg_output={}",
          img.cols, img.rows, inputRange, useJpeg);

    cv::Mat destImg = cv::Mat::zeros(img.size(), img.type());
    std::vector<cv::Mat> bgrPlanes;
    cv::split(img, bgrPlanes);

    LOG_F(INFO, "Split image into {} channels", bgrPlanes.size());

    int maxOutput = useJpeg ? MAX_8BIT_VALUE : MAX_16BIT_VALUE;
    double maxInput = inputRange - 1 > 0 ? inputRange - 1 : inputRange;

    LOG_F(INFO, "Output parameters: max_output={}, max_input={:.2f}", maxOutput,
          maxInput);

    for (int channel = 0; channel < 3; ++channel) {
        LOG_F(INFO, "Processing channel {}/3", channel + 1);

        const auto& params = channelParams[channel];
        LOG_F(INFO,
              "Channel {} parameters: shadows={:.3f}, tones={:.3f}, "
              "highlights={:.3f}",
              channel, params.shadows, params.tones, params.highlights);

        double rangeScale = (params.highlights != params.shadows)
                                ? 1.0 / (params.highlights - params.shadows)
                                : 1.0;

        LOG_F(INFO, "Channel {} range scale: {:.3f}", channel, rangeScale);

        double nativeShadows = params.shadows * maxInput;
        double nativeHighlights = params.highlights * maxInput;
        double factorK1 =
            (params.tones - 1) * rangeScale * maxOutput / maxInput;
        double factorK2 = ((2 * params.tones) - 1) * rangeScale / maxInput;

        LOG_F(INFO, "Channel {} scaling factors: K1={:.3f}, K2={:.3f}", channel,
              factorK1, factorK2);

        int pixelsProcessed = 0;
        int totalPixels = bgrPlanes[channel].rows * bgrPlanes[channel].cols;

        for (int i = 0; i < bgrPlanes[channel].rows; i++) {
            for (int j = 0; j < bgrPlanes[channel].cols; j++) {
                double value = bgrPlanes[channel].at<uchar>(i, j);
                if (value < nativeShadows) {
                    bgrPlanes[channel].at<uchar>(i, j) = 0;
                } else if (value > nativeHighlights) {
                    bgrPlanes[channel].at<uchar>(i, j) = maxOutput;
                } else {
                    bgrPlanes[channel].at<uchar>(i, j) = static_cast<uchar>(
                        ((value - nativeShadows) * factorK1 + EPSILON) /
                        ((value - nativeShadows) * factorK2 - params.tones +
                         EPSILON));
                }

                pixelsProcessed++;
                if (pixelsProcessed % (totalPixels / 10) == 0) {
                    LOG_F(INFO, "Channel {} progress: {:.1f}%", channel,
                          (pixelsProcessed * 100.0) / totalPixels);
                }
            }
        }

        LOG_F(INFO, "Channel {} processing complete", channel);
    }

    cv::merge(bgrPlanes, destImg);
    LOG_F(INFO, "Three channel stretch completed successfully");
    return destImg;
}

auto autoStretch(const cv::Mat& img) -> cv::Mat {
    LOG_F(INFO, "Starting auto stretch");

    cv::Mat result;
    if (img.channels() == 1) {
        auto [shadows, tones, highlights] = calculateStretchParameters(img);
        StretchParams params{shadows, tones, highlights};
        result = stretchOneChannel(img, params);
    } else {
        std::vector<cv::Mat> channels;
        cv::split(img, channels);
        std::vector<cv::Mat> stretched;

        for (const auto& channel : channels) {
            auto [shadows, tones, highlights] =
                calculateStretchParameters(channel);
            StretchParams params{shadows, tones, highlights};
            stretched.push_back(stretchOneChannel(channel, params));
        }

        cv::merge(stretched, result);
    }

    LOG_F(INFO, "Auto stretch completed");
    return result;
}

auto adaptiveStretch(const cv::Mat& img, int blockSize) -> cv::Mat {
    LOG_F(INFO, "Starting adaptive stretch with block size {}", blockSize);

    cv::Mat result = img.clone();
    for (int rowIdx = 0; rowIdx < img.rows; rowIdx += blockSize) {
        for (int colIdx = 0; colIdx < img.cols; colIdx += blockSize) {
            cv::Rect roi(colIdx, rowIdx, std::min(blockSize, img.cols - colIdx),
                         std::min(blockSize, img.rows - rowIdx));
            cv::Mat block = img(roi);
            auto [shadows, tones, highlights] =
                calculateStretchParameters(block);
            StretchParams params{shadows, tones, highlights};
            cv::Mat stretchedBlock = stretchOneChannel(block, params);
            stretchedBlock.copyTo(result(roi));
        }
    }

    LOG_F(INFO, "Adaptive stretch completed");
    return result;
}