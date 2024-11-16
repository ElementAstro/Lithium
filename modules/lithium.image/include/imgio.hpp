#ifndef LITHIUM_MODULE_IMAGE_IMGIO_HPP
#define LITHIUM_MODULE_IMAGE_IMGIO_HPP

#include <string>
#include <vector>

namespace cv {
class Mat;
}
// 加载单张图像
auto loadImage(const std::string& filename, int flags = 1) -> cv::Mat;

// 从文件夹中读取所有图像
auto loadImages(const std::string& folder,
                const std::vector<std::string>& filenames = {},
                int flags = 1) -> std::vector<std::pair<std::string, cv::Mat>>;

// 保存图像到文件
auto saveImage(const std::string& filename, const cv::Mat& image) -> bool;

#endif