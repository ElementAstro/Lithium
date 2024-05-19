#ifndef LITHIUM_IMAGE_DEBAYER_HPP
#define LITHIUM_IMAGE_DEBAYER_HPP

#include <filesystem>
#include <map>
#include <string>
#include <tuple>

#include <opencv2/core.hpp>

std::tuple<cv::Mat, bool, std::map<std::string, std::string>> Debayer(
    const std::filesystem::path& filepath);

#endif
