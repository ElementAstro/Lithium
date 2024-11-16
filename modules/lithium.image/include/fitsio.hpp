#ifndef FITSIO_HPP
#define FITSIO_HPP

#include <filesystem>
#include <map>
#include <opencv2/core.hpp>
#include <optional>
#include <string>

/**
 * @brief Struct to hold the result of reading a FITS file.
 */
struct FitsResult {
    cv::Mat image;  ///< The image data extracted from the FITS file.
    std::map<std::string, std::string> header;  ///< FITS header information.
};

/**
 * @brief Reads a FITS file and converts it to a cv::Mat.
 *
 * 使用 CFITSIO 库读取 FITS 文件并转换为 OpenCV 的 cv::Mat 格式。
 *
 * @param filepath FITS 文件的路径。
 * @return FitsResult 包含图像数据和 FITS 头信息的结构体。
 * @throws std::runtime_error 如果读取过程中发生错误。
 */
FitsResult readFits(const std::filesystem::path& filepath);

/**
 * @brief 将 cv::Mat 图像数据写入 FITS 文件。
 *
 * 使用 CFITSIO 库将 OpenCV 的 cv::Mat 图像数据写入 FITS 文件。
 *
 * @param image 要写入的图像数据。
 * @param filepath 目标 FITS 文件的路径。
 * @throws std::runtime_error 如果写入过程中发生错误。
 */
void writeMatToFits(const cv::Mat& image,
                    const std::filesystem::path& filepath);

/**
 * @brief 将 cv::Mat 图像转换为 Base64 字符串。
 *
 * @param image 要转换的图像数据。
 * @param imgFormat 图像编码格式（如 ".png", ".jpg"）。
 * @return std::string Base64 编码的字符串。
 * @throws std::runtime_error 如果编码过程中发生错误。
 */
std::string matToBase64(const cv::Mat& image, const std::string& imgFormat);

/**
 * @brief 将 FITS 文件转换为 Base64 字符串。
 *
 * @param filepath FITS 文件的路径。
 * @return std::string Base64 编码的字符串。
 * @throws std::runtime_error 如果转换过程中发生错误。
 */
std::string fitsToBase64(const std::filesystem::path& filepath);

/**
 * @brief 从 FITS 文件读取设备名称。
 *
 * @param filepath FITS 文件的路径。
 * @return std::optional<std::string> 如果存在设备名称则返回，否则返回
 * std::nullopt。
 * @throws std::runtime_error 如果读取过程中发生错误。
 */
std::optional<std::string> readFitsDeviceName(
    const std::filesystem::path& filepath);

#endif  // FITSIO_HPP