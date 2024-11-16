#include <concepts>
#include <execution>
#include <memory>
#include <opencv2/opencv.hpp>
#include <span>

#include <boost/iterator/counting_iterator.hpp>

// 定义数值概念
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

struct CamBin {
    uint32_t camxbin{1};
    uint32_t camybin{1};
};

class Tools {
public:
    static constexpr int MAX_IMAGE_SIZE = 2000;

    [[nodiscard]] static auto mergeImageBasedOnSize(const cv::Mat& image)
        -> CamBin {
        const int WIDTH = image.cols;
        const int HEIGHT = image.rows;

        CamBin result;

        // 使用 C++20 constexpr if 简化逻辑
        if (WIDTH > MAX_IMAGE_SIZE || HEIGHT > MAX_IMAGE_SIZE) {
            constexpr std::array<int, 3> BIN_SIZES{2, 3, 4};
            for (const auto BIN : BIN_SIZES) {
                if (WIDTH / BIN <= MAX_IMAGE_SIZE &&
                    HEIGHT / BIN <= MAX_IMAGE_SIZE) {
                    result.camxbin = result.camybin =
                        static_cast<uint32_t>(BIN);
                    break;
                }
            }
        }

        return result;
    }

    [[nodiscard]] static cv::Mat processMatWithBinAvg(const cv::Mat& image,
                                                      uint32_t camxbin,
                                                      uint32_t camybin,
                                                      bool isColor = false,
                                                      bool isAVG = true) {
        CV_Assert(!image.empty() && camxbin > 0 && camybin > 0);

        const uint32_t WIDTH = image.cols;
        const uint32_t HEIGHT = image.rows;
        const uint32_t DEPTH = image.elemSize() * 8;
        const uint32_t CHANNELS = image.channels();

        // 使用 std::span 处理图像数据
        std::span<const uint8_t> srcSpan{image.data,
                                         image.total() * image.elemSize()};

        // 计算新尺寸
        const uint32_t NEW_WIDTH = WIDTH / camxbin;
        const uint32_t NEW_HEIGHT = HEIGHT / camybin;

        // 创建输出图像
        cv::Mat result;

        if (isAVG) {
            result =
                processWithAverage(srcSpan, WIDTH, HEIGHT, DEPTH, NEW_WIDTH,
                                   NEW_HEIGHT, camxbin, camybin);
        } else {
            result = processWithBinning(srcSpan, WIDTH, HEIGHT, CHANNELS, DEPTH,
                                        NEW_WIDTH, NEW_HEIGHT, camxbin, camybin,
                                        isColor);
        }

        return result;
    }

private:
    template <Numeric T>
    static auto calculateAverage(std::span<const T> values,
                                 size_t binSize) -> T {
        return std::reduce(std::execution::par_unseq, values.begin(),
                           values.end(), T{}) /
               binSize;
    }

    [[nodiscard]] static cv::Mat processWithAverage(
        std::span<const uint8_t> srcData, uint32_t width, uint32_t height,
        uint32_t depth, uint32_t newWidth, uint32_t newHeight, uint32_t camxbin,
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
    static void parallel_process_bin(std::span<const uint8_t> srcData,
                                     cv::Mat& result, uint32_t width,
                                     uint32_t height, uint32_t camxbin,
                                     uint32_t camybin, uint32_t binArea) {
        const uint32_t NEW_WIDTH = width / camxbin;
        const uint32_t NEW_HEIGHT = height / camybin;

        // 并行处理每一行
        std::for_each(
            std::execution::par_unseq, boost::counting_iterator<uint32_t>(0),
            boost::counting_iterator<uint32_t>(NEW_HEIGHT), [&](uint32_t y) {
                for (uint32_t x = 0; x < NEW_WIDTH; ++x) {
                    T sum = 0;
                    // 处理每个bin区域
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

    [[nodiscard]] static cv::Mat processWithBinning(
        std::span<const uint8_t> srcData, uint32_t width, uint32_t height,
        uint32_t channels, uint32_t depth, uint32_t newWidth,
        uint32_t newHeight, uint32_t camxbin, uint32_t camybin, bool isColor) {
        // Handle color images
        if (isColor) {
            cv::Mat srcMat(height, width, depth == 8 ? CV_8UC3 : CV_16UC3,
                           const_cast<uint8_t*>(srcData.data()));

            cv::Mat dstMat(newHeight, newWidth, srcMat.type());
            cv::resize(srcMat, dstMat, dstMat.size(), 0, 0, cv::INTER_AREA);

            return dstMat;
        }

        // Handle monochrome images
        cv::Mat result;
        const uint32_t elemSize = depth / 8;

        switch (depth) {
            case 8:
                result = cv::Mat::zeros(newHeight, newWidth, CV_8U);
                process_mono_bin<uint8_t>(srcData, result, width, camxbin,
                                          camybin);
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
    static void process_mono_bin(std::span<const uint8_t> srcData,
                                 cv::Mat& result, uint32_t srcStride,
                                 uint32_t camxbin, uint32_t camybin) {
        const uint32_t NEW_WIDTH = result.cols;
        const uint32_t NEW_HEIGHT = result.rows;

        std::for_each(
            std::execution::par_unseq, boost::counting_iterator<uint32_t>(0),
            boost::counting_iterator<uint32_t>(NEW_HEIGHT), [&](uint32_t y) {
                for (uint32_t x = 0; x < NEW_WIDTH; ++x) {
                    T sum = 0;
                    const auto* src =
                        reinterpret_cast<const T*>(srcData.data());

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
                        result.at<T>(y, x) = static_cast<T>(std::min<uint32_t>(
                            sum, std::numeric_limits<T>::max()));
                    }
                }
            });
    }
};