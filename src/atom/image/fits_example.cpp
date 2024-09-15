#include <iomanip>
#include <iostream>
#include "fits_file.hpp"

int main() {
    try {
        FITSFile fitsFile;

        // 创建一个简单的 10x10 彩色图像
        auto imageHDU = std::make_unique<ImageHDU>();
        imageHDU->setImageSize(10, 10, 3);  // 3 channels for RGB
        imageHDU->setHeaderKeyword("SIMPLE", "T");
        imageHDU->setHeaderKeyword("BITPIX", "16");
        imageHDU->setHeaderKeyword("NAXIS", "3");
        imageHDU->setHeaderKeyword("EXTEND", "T");

        // 用渐变填充图像
        for (int y = 0; y < 10; ++y) {
            for (int x = 0; x < 10; ++x) {
                imageHDU->setPixel<int16_t>(x, y,
                                            static_cast<int16_t>(x * 1000 / 9),
                                            0);  // Red channel
                imageHDU->setPixel<int16_t>(x, y,
                                            static_cast<int16_t>(y * 1000 / 9),
                                            1);  // Green channel
                imageHDU->setPixel<int16_t>(
                    x, y, static_cast<int16_t>((x + y) * 500 / 9),
                    2);  // Blue channel
            }
        }

        fitsFile.addHDU(std::move(imageHDU));

        // 写入文件
        fitsFile.writeFITS("test_color.fits");

        // 读取文件
        FITSFile readFile;
        readFile.readFITS("test_color.fits");

        // 验证图像内容
        const auto& readHDU = dynamic_cast<const ImageHDU&>(readFile.getHDU(0));
        auto [width, height, channels] = readHDU.getImageSize();
        std::cout << "Image size: " << width << "x" << height << "x" << channels
                  << std::endl;

        // 显示每个通道的第一行
        for (int c = 0; c < channels; ++c) {
            std::cout << "Channel " << c << ", first row:" << std::endl;
            for (int x = 0; x < width; ++x) {
                std::cout << std::setw(5) << readHDU.getPixel<int16_t>(x, 0, c)
                          << " ";
            }
            std::cout << std::endl;
        }

        // 计算每个通道的图像统计信息
        for (int c = 0; c < channels; ++c) {
            auto stats = readHDU.computeImageStats<int16_t>(c);
            std::cout << "\nImage statistics for channel " << c << ":"
                      << std::endl;
            std::cout << "Min: " << stats.min << std::endl;
            std::cout << "Max: " << stats.max << std::endl;
            std::cout << "Mean: " << stats.mean << std::endl;
            std::cout << "StdDev: " << stats.stddev << std::endl;
        }

        // 应用高斯模糊滤波器到绿色通道
        std::vector<std::vector<double>> gaussianKernel = {
            {1 / 16.0, 1 / 8.0, 1 / 16.0},
            {1 / 8.0, 1 / 4.0, 1 / 8.0},
            {1 / 16.0, 1 / 8.0, 1 / 16.0}};

        auto& editableHDU = dynamic_cast<ImageHDU&>(readFile.getHDU(0));
        editableHDU.applyFilter<int16_t>(gaussianKernel,
                                         1);  // Apply to green channel only

        std::cout << "\nAfter applying Gaussian blur to green channel:"
                  << std::endl;
        for (int c = 0; c < channels; ++c) {
            std::cout << "Channel " << c << ", first row:" << std::endl;
            for (int x = 0; x < width; ++x) {
                std::cout << std::setw(5)
                          << editableHDU.getPixel<int16_t>(x, 0, c) << " ";
            }
            std::cout << std::endl;
        }

        // 将修改后的图像保存到新文件
        readFile.writeFITS("test_color_blurred.fits");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
