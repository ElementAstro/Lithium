#include "centroid.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

// 命名常量定义
constexpr double EPSILON_TOLERANCE = 1e-10;
constexpr float SUBPIXEL_WEIGHT = 4.0F;
constexpr int CIRCLE_RADIUS = 3;
constexpr int CIRCLE_THICKNESS = 1;
constexpr int NORMALIZE_MIN = 0;
constexpr int NORMALIZE_MAX = 255;
constexpr int ROI_SIZE = 3;
constexpr float ROI_CENTER = 1.0F;
constexpr float ROI_OFFSET = 1.0F;

auto StarCentroid::readFits(const std::string& filename) -> cv::Mat {
    LOG_F(INFO, "Reading FITS file: {}", filename);
    try {
        cv::Mat image = cv::imread(filename, cv::IMREAD_ANYDEPTH);
        if (image.empty()) {
            LOG_F(ERROR, "Failed to read image file: {}", filename);
            THROW_RUNTIME_ERROR("Image reading failed");
        }
        image.convertTo(image, CV_32F);
        LOG_F(INFO, "Successfully read and converted image file: {}", filename);
        return image;
    } catch (const cv::Exception& e) {
        LOG_F(ERROR, "OpenCV exception while reading image: {}", e.what());
        throw;
    }
}

auto StarCentroid::calcIntensityWeightedCenter(const cv::Mat& image)
    -> cv::Point2f {
    LOG_F(INFO, "Calculating intensity-weighted center");
    CV_Assert(image.type() == CV_32F);

    cv::Mat moments;
    cv::moments(image, true);

    double m00 = moments.at<double>(0, 0);
    double m10 = moments.at<double>(1, 0);
    double m01 = moments.at<double>(0, 1);

    LOG_F(INFO, "Moments calculated: m00={}, m10={}, m01={}", m00, m10, m01);

    if (std::abs(m00) < EPSILON_TOLERANCE) {
        LOG_F(WARNING, "Zero or very small total intensity");
        return {0.0F, 0.0F};
    }

    cv::Point2f center = {static_cast<float>(m10 / m00),
                          static_cast<float>(m01 / m00)};
    LOG_F(INFO, "Intensity-weighted center: ({}, {})", center.x, center.y);
    return center;
}

auto StarCentroid::calcSubPixelCenter(const cv::Mat& roi,
                                      cv::Point2f&& initCenter, float epsilon,
                                      int maxIterations) -> cv::Point2f {
    LOG_F(INFO, "Calculating sub-pixel center");
    CV_Assert(roi.type() == CV_32F);
    CV_Assert(roi.rows == ROI_SIZE && roi.cols == ROI_SIZE);

    cv::Point2f center = std::move(initCenter);

#pragma unroll
    for (int iter = 0; iter < maxIterations; ++iter) {
        float centerVal = roi.at<float>(1, 1);
        float centerValDouble = 2.0F * centerVal;

        std::array<float, 4> subPixels{
            (roi.at<float>(1, 0) + roi.at<float>(0, 1) + centerValDouble) /
                SUBPIXEL_WEIGHT,
            (roi.at<float>(1, 2) + roi.at<float>(0, 1) + centerValDouble) /
                SUBPIXEL_WEIGHT,
            (roi.at<float>(1, 2) + roi.at<float>(2, 1) + centerValDouble) /
                SUBPIXEL_WEIGHT,
            (roi.at<float>(1, 0) + roi.at<float>(2, 1) + centerValDouble) /
                SUBPIXEL_WEIGHT};

        auto* maxElement = std::max_element(subPixels.begin(), subPixels.end());
        auto maxIndex =
            static_cast<int>(std::distance(subPixels.begin(), maxElement));

        auto shiftAmount = static_cast<float>(std::pow(2.0, -(iter + 1)));

        const std::array<cv::Point2f, 4> K_DIRECTIONS{
            {{-shiftAmount, -shiftAmount},
             {shiftAmount, -shiftAmount},
             {shiftAmount, shiftAmount},
             {-shiftAmount, shiftAmount}}};

        cv::Point2f oldCenter = center;
        center += K_DIRECTIONS[maxIndex];

        LOG_F(INFO, "Iteration {}: center moved to ({}, {})", iter, center.x,
              center.y);

        if (cv::norm(center - oldCenter) < epsilon) {
            LOG_F(INFO, "Sub-pixel convergence reached at iteration {}", iter);
            break;
        }
    }

    LOG_F(INFO, "Final sub-pixel center: ({}, {})", center.x, center.y);
    return center;
}

auto StarCentroid::findCentroid(const cv::Mat& image) -> CentroidResult {
    LOG_F(INFO, "Finding centroid");
    CentroidResult result;

    result.weightedCenter = calcIntensityWeightedCenter(image);
    result.roundedCenter = cv::Point2i(cvRound(result.weightedCenter.x),
                                       cvRound(result.weightedCenter.y));

    LOG_F(INFO, "Weighted center: ({}, {}), Rounded center: ({}, {})",
          result.weightedCenter.x, result.weightedCenter.y,
          result.roundedCenter.x, result.roundedCenter.y);

    cv::Rect roi(result.roundedCenter.x - 1, result.roundedCenter.y - 1,
                 ROI_SIZE, ROI_SIZE);
    roi &= cv::Rect(0, 0, image.cols, image.rows);

    if (roi.width != ROI_SIZE || roi.height != ROI_SIZE) {
        LOG_F(ERROR, "Unable to extract 3x3 ROI - too close to image edge");
        result.subPixelCenter = result.weightedCenter;
        return result;
    }

    cv::Mat roiMat = image(roi);
    cv::Point2f localCenter{ROI_CENTER, ROI_CENTER};
    cv::Point2f refinedLocal =
        calcSubPixelCenter(roiMat, std::move(localCenter));

    result.subPixelCenter = {static_cast<float>(result.roundedCenter.x) -
                                 ROI_OFFSET + refinedLocal.x,
                             static_cast<float>(result.roundedCenter.y) -
                                 ROI_OFFSET + refinedLocal.y};

    LOG_F(INFO, "Sub-pixel center: ({}, {})", result.subPixelCenter.x,
          result.subPixelCenter.y);
    return result;
}

void visualizeResults(const cv::Mat& image,
                      const StarCentroid::CentroidResult& result) {
    LOG_F(INFO, "Visualizing results");
    cv::Mat display;
    cv::normalize(image, display, NORMALIZE_MIN, NORMALIZE_MAX,
                  cv::NORM_MINMAX);
    display.convertTo(display, CV_8U);
    cv::cvtColor(display, display, cv::COLOR_GRAY2BGR);

    cv::circle(display, result.weightedCenter, CIRCLE_RADIUS,
               cv::Scalar(0, NORMALIZE_MAX, 0), CIRCLE_THICKNESS);
    cv::circle(display, result.subPixelCenter, CIRCLE_RADIUS,
               cv::Scalar(0, 0, NORMALIZE_MAX), CIRCLE_THICKNESS);
    cv::circle(display, result.roundedCenter, CIRCLE_RADIUS,
               cv::Scalar(NORMALIZE_MAX, 0, 0), CIRCLE_THICKNESS);

    cv::imshow("Star Centroid", display);
    cv::waitKey(0);
    LOG_F(INFO, "Results visualized");
}