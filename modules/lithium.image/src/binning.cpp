#include "binning.hpp"

#include <algorithm>
#include <numeric>

constexpr int MAX_IMAGE_SIZE = 2000;

CamBin mergeImageBasedOnSize(const cv::Mat& image) {
    const int WIDTH = image.cols;
    const int HEIGHT = image.rows;

    CamBin result;

    if (WIDTH > MAX_IMAGE_SIZE || HEIGHT > MAX_IMAGE_SIZE) {
        constexpr std::array<int, 3> BIN_SIZES{2, 3, 4};
        for (const auto BIN : BIN_SIZES) {
            if (WIDTH / BIN <= MAX_IMAGE_SIZE &&
                HEIGHT / BIN <= MAX_IMAGE_SIZE) {
                result.camxbin = result.camybin = static_cast<uint32_t>(BIN);
                break;
            }
        }
    }

    return result;
}

cv::Mat processMatWithBinAvg(const cv::Mat& image, uint32_t camxbin,
                             uint32_t camybin, bool isColor, bool isAVG) {
    CV_Assert(!image.empty() && camxbin > 0 && camybin > 0);

    const uint32_t WIDTH = image.cols;
    const uint32_t HEIGHT = image.rows;
    const uint32_t DEPTH = image.elemSize() * 8;
    const uint32_t CHANNELS = image.channels();

    std::span<const uint8_t> srcSpan{image.data,
                                     image.total() * image.elemSize()};

    const uint32_t NEW_WIDTH = WIDTH / camxbin;
    const uint32_t NEW_HEIGHT = HEIGHT / camybin;

    cv::Mat result;

    if (isAVG) {
        result = processWithAverage(srcSpan, WIDTH, HEIGHT, DEPTH, NEW_WIDTH,
                                    NEW_HEIGHT, camxbin, camybin);
    } else {
        result = processWithBinning(srcSpan, WIDTH, HEIGHT, CHANNELS, DEPTH,
                                    NEW_WIDTH, NEW_HEIGHT, camxbin, camybin,
                                    isColor);
    }

    return result;
}

template <typename T>
T calculateAverage(std::span<const T> values, size_t binSize) {
    return std::reduce(values.begin(), values.end(), T{}) / binSize;
}

cv::Mat processWithAverage(std::span<const uint8_t> srcData, uint32_t width,
                           uint32_t height, uint32_t depth, uint32_t newWidth,
                           uint32_t newHeight, uint32_t camxbin,
                           uint32_t camybin) {
    cv::Mat result;
    const uint32_t BIN_AREA = camxbin * camybin;

    switch (depth) {
        case 8: {
            result = cv::Mat::zeros(newHeight, newWidth, CV_8U);
            parallel_process_bin<uint8_t>(srcData, result, width, height,
                                          camxbin, camybin, BIN_AREA);
            break;
        }
        case 16: {
            result = cv::Mat::zeros(newHeight, newWidth, CV_16U);
            parallel_process_bin<uint16_t>(srcData, result, width, height,
                                           camxbin, camybin, BIN_AREA);
            break;
        }
        case 32: {
            result = cv::Mat::zeros(newHeight, newWidth, CV_32S);
            parallel_process_bin<uint32_t>(srcData, result, width, height,
                                           camxbin, camybin, BIN_AREA);
            break;
        }
        default:
            throw std::runtime_error("Unsupported bit depth");
    }

    return result;
}

template <typename T>
void parallel_process_bin(std::span<const uint8_t> srcData, cv::Mat& result,
                          uint32_t width, uint32_t height, uint32_t camxbin,
                          uint32_t camybin, uint32_t binArea) {
    const uint32_t NEW_WIDTH = width / camxbin;
    const uint32_t NEW_HEIGHT = height / camybin;

    std::for_each(
        boost::counting_iterator<uint32_t>(0),
        boost::counting_iterator<uint32_t>(NEW_HEIGHT), [&](uint32_t y) {
            for (uint32_t x = 0; x < NEW_WIDTH; ++x) {
                T sum = 0;
                for (uint32_t by = 0; by < camybin; ++by) {
                    for (uint32_t bx = 0; bx < camxbin; ++bx) {
                        const uint32_t SRC_X = x * camxbin + bx;
                        const uint32_t SRC_Y = y * camybin + by;
                        const auto* src =
                            reinterpret_cast<const T*>(srcData.data());
                        sum += src[SRC_Y * width + SRC_X];
                    }
                }
                result.at<T>(y, x) = static_cast<T>(sum / binArea);
            }
        });
}

cv::Mat processWithBinning(std::span<const uint8_t> srcData, uint32_t width,
                           uint32_t height, uint32_t channels, uint32_t depth,
                           uint32_t newWidth, uint32_t newHeight,
                           uint32_t camxbin, uint32_t camybin, bool isColor) {
    if (isColor) {
        cv::Mat srcMat(height, width, depth == 8 ? CV_8UC3 : CV_16UC3,
                       const_cast<uint8_t*>(srcData.data()));
        cv::Mat dstMat(newHeight, newWidth, srcMat.type());
        cv::resize(srcMat, dstMat, dstMat.size(), 0, 0, cv::INTER_AREA);
        return dstMat;
    }

    cv::Mat result;
    const uint32_t elemSize = depth / 8;

    switch (depth) {
        case 8:
            result = cv::Mat::zeros(newHeight, newWidth, CV_8U);
            process_mono_bin<uint8_t>(srcData, result, width, camxbin, camybin);
            break;
        case 16:
            result = cv::Mat::zeros(newHeight, newWidth, CV_16U);
            process_mono_bin<uint16_t>(srcData, result, width, camxbin,
                                       camybin);
            break;
        case 32:
            result = cv::Mat::zeros(newHeight, newWidth, CV_32S);
            process_mono_bin<uint32_t>(srcData, result, width, camxbin,
                                       camybin);
            break;
        default:
            throw std::runtime_error("Unsupported bit depth");
    }

    return result;
}

template <typename T>
void process_mono_bin(std::span<const uint8_t> srcData, cv::Mat& result,
                      uint32_t srcStride, uint32_t camxbin, uint32_t camybin) {
    const uint32_t NEW_WIDTH = result.cols;
    const uint32_t NEW_HEIGHT = result.rows;

    std::for_each(
        boost::counting_iterator<uint32_t>(0),
        boost::counting_iterator<uint32_t>(NEW_HEIGHT), [&](uint32_t y) {
            for (uint32_t x = 0; x < NEW_WIDTH; ++x) {
                T sum = 0;
                const auto* src = reinterpret_cast<const T*>(srcData.data());

                for (uint32_t by = 0; by < camybin; ++by) {
                    for (uint32_t bx = 0; bx < camxbin; ++bx) {
                        const uint32_t SRC_X = x * camxbin + bx;
                        const uint32_t SRC_Y = y * camybin + by;
                        sum += src[SRC_Y * srcStride + SRC_X];
                    }
                }

                if constexpr (std::is_same_v<T, uint32_t>) {
                    result.at<T>(y, x) = sum;
                } else {
                    result.at<T>(y, x) = static_cast<T>(
                        std::min<uint32_t>(sum, std::numeric_limits<T>::max()));
                }
            }
        });
}

template void parallel_process_bin<uint8_t>(std::span<const uint8_t> srcData,
                                            cv::Mat& result, uint32_t width,
                                            uint32_t height, uint32_t camxbin,
                                            uint32_t camybin, uint32_t binArea);
template void parallel_process_bin<uint16_t>(std::span<const uint8_t> srcData,
                                             cv::Mat& result, uint32_t width,
                                             uint32_t height, uint32_t camxbin,
                                             uint32_t camybin,
                                             uint32_t binArea);
template void parallel_process_bin<uint32_t>(std::span<const uint8_t> srcData,
                                             cv::Mat& result, uint32_t width,
                                             uint32_t height, uint32_t camxbin,
                                             uint32_t camybin,
                                             uint32_t binArea);

template void process_mono_bin<uint8_t>(std::span<const uint8_t> srcData,
                                        cv::Mat& result, uint32_t srcStride,
                                        uint32_t camxbin, uint32_t camybin);
template void process_mono_bin<uint16_t>(std::span<const uint8_t> srcData,
                                         cv::Mat& result, uint32_t srcStride,
                                         uint32_t camxbin, uint32_t camybin);
template void process_mono_bin<uint32_t>(std::span<const uint8_t> srcData,
                                         cv::Mat& result, uint32_t srcStride,
                                         uint32_t camxbin, uint32_t camybin);