#ifndef LITHIUM_IMAGE_FITSIO_HPP
#define LITHIUM_IMAGE_FITSIO_HPP

#include <filesystem>
#include <opencv2/core.hpp>
#include <string>

void checkFitsStatus(int status, const std::string& errorMessage);
auto readFitsToMat(const std::filesystem::path& filepath) -> cv::Mat;
void writeMatToFits(const cv::Mat& image,
                    const std::filesystem::path& filepath);
auto matToBase64(const cv::Mat& image,
                 const std::string& imgFormat) -> std::string;
auto fitsToBase64(const std::filesystem::path& filepath) -> std::string;
auto readFitsHeadForDevName(const std::string& filename);
auto readFits(const std::string& fileName, cv::Mat& image) -> int;
auto readFits_(const std::string& fileName, cv::Mat& image) -> int;

#endif
