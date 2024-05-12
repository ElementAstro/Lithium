#ifndef LITHIUM_IMAGE_FITSIO_HPP
#define LITHIUM_IMAGE_FITSIO_HPP

#include <filesystem>
#include <opencv2/core.hpp>
#include <string>


void checkFitsStatus(int status, const std::string& errorMessage);
cv::Mat readFitsToMat(const std::filesystem::path& filepath);
void writeMatToFits(const cv::Mat& image,
                    const std::filesystem::path& filepath);
std::string matToBase64(const cv::Mat& image, const std::string& imgFormat);
std::string fitsToBase64(const std::filesystem::path& filepath);

#endif
