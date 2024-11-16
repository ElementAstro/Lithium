#include "imgio.hpp"

#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

// 加载单张图像
auto loadImage(const std::string& filename,
               int flags) -> cv::Mat {
    cv::Mat image = cv::imread(filename, flags);
    if (image.empty()) {
        LOG_F(ERROR, "Failed to load image: {}", filename);
        return {};
    }
    return image;
}

// 从文件夹中读取所有图像
auto loadImages(const std::string& folder,
                const std::vector<std::string>& filenames,
                int flags)
    -> std::vector<std::pair<std::string, cv::Mat>> {
    std::vector<std::pair<std::string, cv::Mat>> images;

    if (filenames.empty()) {
        // 遍历整个文件夹
        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                cv::Mat img = cv::imread(filepath, flags);
                if (!img.empty()) {
                    images.emplace_back(filepath, img);
                } else {
                    LOG_F(ERROR, "Failed to load image: {}", filepath);
                }
            }
        }
    } else {
        // 读取指定文件名的图像
        for (const auto& filename : filenames) {
            std::string filepath = folder + "/" + filename;
            cv::Mat img = cv::imread(filepath, flags);
            if (!img.empty()) {
                images.emplace_back(filepath, img);
            } else {
                LOG_F(ERROR, "Failed to load image: {}", filepath);
            }
        }
    }

    return images;
}

// 保存图像到文件
auto saveImage(const std::string& filename, const cv::Mat& image) -> bool {
    if (cv::imwrite(filename, image)) {
        LOG_F(INFO, "Image saved successfully: {}", filename);
        return true;
    }
    LOG_F(ERROR, "Failed to save image: {}", filename);
    return false;
}