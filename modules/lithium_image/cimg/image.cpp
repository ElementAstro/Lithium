/*
 * image.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-1

Description: Image processing plugin for Lithium

**************************************************/

#include "image.hpp"

#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"

ImageProcessingPlugin::ImageProcessingPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : Plugin(path, version, author, description)
{
    // 在构造函数中注册图像处理函数
    RegisterFunc("blur", &ImageProcessingPlugin::Blur, this);
    RegisterFunc("rotate", &ImageProcessingPlugin::Rotate, this);
    RegisterFunc("crop", &ImageProcessingPlugin::Crop, this);
    RegisterFunc("sharpen", &ImageProcessingPlugin::Sharpen, this);
    RegisterFunc("white_balance", &ImageProcessingPlugin::WhiteBalance, this);
    RegisterFunc("resize", &ImageProcessingPlugin::Resize, this);
    RegisterFunc("blur", &ImageProcessingPlugin::Blur, this);
    RegisterFunc("rotate", &ImageProcessingPlugin::Rotate, this);
    RegisterFunc("image2base64", &ImageProcessingPlugin::ImageToBase64, this);
    RegisterFunc("base642image", &ImageProcessingPlugin::Base64ToImage, this);
}

void ImageProcessingPlugin::Blur(const json &params)
{
    std::string imagePath = params[0].get<std::string>();
    int radius = params[1].get<int>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行模糊处理
        CImg<unsigned char> image = imageCache[imagePath];
        image.blur(radius);
        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行模糊处理
        CImg<unsigned char> image(imagePath.c_str());
        image.blur(radius);
        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image blurred: " << imagePath << std::endl;
}

void ImageProcessingPlugin::Rotate(const json &params)
{
    std::string imagePath = params[0].get<std::string>();
    int angle = params[1].get<int>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行旋转
        CImg<unsigned char> image = imageCache[imagePath];
        image.rotate(angle);
        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行旋转
        CImg<unsigned char> image(imagePath.c_str());
        image.rotate(angle);
        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image rotated: " << imagePath << std::endl;
}

void ImageProcessingPlugin::Crop(const json &params)
{
    std::string imagePath = params[0].get<std::string>();
    int x = params[1].get<int>();
    int y = params[2].get<int>();
    int width = params[3].get<int>();
    int height = params[4].get<int>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行裁剪
        CImg<unsigned char> image = imageCache[imagePath];
        image.crop(x, y, x + width - 1, y + height - 1);
        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行裁剪
        CImg<unsigned char> image(imagePath.c_str());
        image.crop(x, y, x + width - 1, y + height - 1);
        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image cropped: " << imagePath << std::endl;
}

void ImageProcessingPlugin::Sharpen(const json &params)
{
    std::string imagePath = params[0].get<std::string>();
    int factor = params[1].get<int>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行锐化
        CImg<unsigned char> image = imageCache[imagePath];
        image.sharpen(factor);
        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行锐化
        CImg<unsigned char> image(imagePath.c_str());
        image.sharpen(factor);
        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image sharpened: " << imagePath << std::endl;
}

void ImageProcessingPlugin::WhiteBalance(const json &params)
{
    std::string imagePath = params[0].get<std::string>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行白平衡调整
        CImg<unsigned char> image = imageCache[imagePath];

        // 计算每个通道的平均值
        double r = 0, g = 0, b = 0;
        cimg_forXY(image, x, y)
        {
            r += image(x, y, 0);
            g += image(x, y, 1);
            b += image(x, y, 2);
        }
        int size = image.width() * image.height();
        r /= size;
        g /= size;
        b /= size;

        // 调整每个通道的白平衡
        cimg_forXY(image, x, y)
        {
            double factor_r = r / image(x, y, 0);
            double factor_g = g / image(x, y, 1);
            double factor_b = b / image(x, y, 2);
            image(x, y, 0) = cimg::cut(image(x, y, 0) * factor_r, 0, 255);
            image(x, y, 1) = cimg::cut(image(x, y, 1) * factor_g, 0, 255);
            image(x, y, 2) = cimg::cut(image(x, y, 2) * factor_b, 0, 255);
        }

        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行白平衡调整
        CImg<unsigned char> image(imagePath.c_str());

        // 计算每个通道的平均值
        double r = 0, g = 0, b = 0;
        cimg_forXY(image, x, y)
        {
            r += image(x, y, 0);
            g += image(x, y, 1);
            b += image(x, y, 2);
        }
        int size = image.width() * image.height();
        r /= size;
        g /= size;
        b /= size;

        // 调整每个通道的白平衡
        cimg_forXY(image, x, y)
        {
            double factor_r = r / image(x, y, 0);
            double factor_g = g / image(x, y, 1);
            double factor_b = b / image(x, y, 2);
            image(x, y, 0) = cimg::cut(image(x, y, 0) * factor_r, 0, 255);
            image(x, y, 1) = cimg::cut(image(x, y, 1) * factor_g, 0, 255);
            image(x, y, 2) = cimg::cut(image(x, y, 2) * factor_b, 0, 255);
        }

        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image white balanced: " << imagePath << std::endl;
}

void ImageProcessingPlugin::Resize(const json &params)
{
    std::string imagePath = params[0].get<std::string>();
    int width = params[1].get<int>();
    int height = params[2].get<int>();

    // 检查缓存中是否存在图像
    if (imageCache.find(imagePath) != imageCache.end())
    {
        // 从缓存中获取图像并进行大小调整
        CImg<unsigned char> image = imageCache[imagePath];
        image.resize(width, height);
        imageCache[imagePath] = image;
    }
    else
    {
        // 加载图像，并进行大小调整
        CImg<unsigned char> image(imagePath.c_str());
        image.resize(width, height);
        imageCache[imagePath] = image;
    }

    // 在这里可以根据实际需求对图像进行进一步处理或保存
    // ...

    // 输出处理后的图像信息
    std::cout << "Image resized: " << imagePath << std::endl;
}

void ImageProcessingPlugin::ImageToBase64(const json &params)
{
    // 读取图片
    try
    {
        std::string imagePath = params["path"].get<std::string>();

        CImg<unsigned char> image(imagePath.c_str());

        // 将图片数据转为字符串
        std::ostringstream oss;
        //image.save(oss, "jpg");
        std::string imageData = oss.str();

        // 将图片数据进行Base64编码
        std::string base64Data;
        const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int currentIndex = 0;
        int padding = 0;
        unsigned int value = 0;
        for (char c : imageData)
        {
            value = (value << 8) | c;
            padding += 8;

            while (padding >= 6)
            {
                padding -= 6;
                unsigned int index = (value >> padding) & 0x3F;
                base64Data += base64Chars[index];
            }
        }

        // 处理剩余的位数
        if (padding > 0)
        {
            value <<= 6 - padding;
            unsigned int index = value & 0x3F;
            base64Data += base64Chars[index];
            padding += 2;
            while (padding < 8)
            {
                base64Data += '=';
                padding += 2;
            }
        }
    }
    catch (const json::parse_error &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void ImageProcessingPlugin::Base64ToImage(const json &params)
{
    // 将Base64解码为图片数据
    std::string base64Data = params["base64"].get<std::string>();
    std::string outputPath = params["path"].get<std::string>();
    std::string imageData;
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int currentIndex = 0;
    unsigned int value = 0;
    int padding = 0;
    for (char c : base64Data)
    {
        if (c == '=')
        {
            padding++;
            continue;
        }
        value = (value << 6) | base64Chars.find(c);
        padding--;
        if (padding == 0)
        {
            imageData += (value >> 16) & 0xFF;
            imageData += (value >> 8) & 0xFF;
            imageData += value & 0xFF;
            value = 0;
        }
    }

    // 创建CImg对象并保存为图片文件
    std::istringstream iss(imageData);
    CImg<unsigned char> image;
    //image.load(iss);

    image.save(outputPath.c_str());
}

double ImageProcessingPlugin::calc_hfd(const CImg<unsigned char> &img, int outer_diameter)
{
    if (outer_diameter == 0)
    {
        outer_diameter = 60;
    }

    // 计算图像的均值
    double mean = img.mean();

    // 对图像进行处理
    CImg<unsigned char> output = img;
    cimg_forXY(img, x, y)
    {
        if (img(x, y) < mean)
        {
            output(x, y) = 0;
        }
        else
        {
            output(x, y) -= static_cast<unsigned char>(mean);
        }
    }

    // 计算外圆直径
    double out_radius = static_cast<double>(outer_diameter) / 2;

    // 计算中心点坐标
    int center_x = img.width() / 2;
    int center_y = img.height() / 2;

    // 计算 HFD
    double _sum = 0, sum_dist = 0;
    cimg_forXY(img, x, y)
    {
        if (pow(x - center_x, 2) + pow(y - center_y, 2) <= pow(out_radius, 2))
        {
            _sum += (output(x, y) != 0);
            sum_dist += output(x, y) * sqrt(pow(x - center_x, 2) + pow(y - center_y, 2));
        }
    }

    if (_sum != 0)
    {
        return 2 * sum_dist / _sum;
    }
    else
    {
        return sqrt(2) * out_radius;
    }
}

void ImageProcessingPlugin::calc_dark_noise(const CImg<unsigned char> &dark_img, float &average_dark, float &sigma_dark, float &sigma_readout)
{
    // 计算均值
    average_dark = 0;
    cimg_forXY(dark_img, x, y)
    {
        average_dark += dark_img(x, y);
    }
    average_dark /= (dark_img.width() * dark_img.height());

    // 计算标准差
    sigma_dark = 0;
    sigma_readout = 0;
    cimg_forXY(dark_img, x, y)
    {
        sigma_dark += pow(dark_img(x, y) - average_dark, 2);
        if (x < dark_img.width() - 1)
        {
            sigma_readout += pow(dark_img(x, y) - dark_img(x + 1, y), 2);
        }
        if (y < dark_img.height() - 1)
        {
            sigma_readout += pow(dark_img(x, y) - dark_img(x, y + 1), 2);
        }
    }
    sigma_dark = sqrt(sigma_dark / (dark_img.width() * dark_img.height()));
    sigma_readout = sqrt(sigma_readout / (2 * (dark_img.width() - 1) * dark_img.height() + 2 * dark_img.width() * (dark_img.height() - 1)));
}

void ImageProcessingPlugin::detect_stars(const char *filename, int threshold, int max_radius)
{
    // 读取图像并记录日志
    DLOG_F(INFO, "Loading image: {}", filename);
    CImg<unsigned char> img(filename);

    // 转换成灰度图像
    CImg<unsigned char> gray = img.get_RGBtoYCbCr().get_channel(0);

    // 阈值处理
    CImg<unsigned char> binary = gray.threshold(threshold);

    // 星点检测
    CImg<unsigned char> stars(binary.width(), binary.height(), 1, 1, 0);
    int count = 0;
    cimg_forXY(binary, x, y)
    {
        if (binary(x, y) == 0)
        {
            for (int r = 1; r <= max_radius; r++)
            {
                bool is_star = true;
                for (int t = 0; t < stars.spectrum(); t++)
                {
                    int tx = x + r * std::cos(t * cimg::PI / 4);
                    int ty = y + r * std::sin(t * cimg::PI / 4);
                    if (tx < 0 || tx >= binary.width() || ty < 0 || ty >= binary.height() || binary(tx, ty) != 0)
                    {
                        is_star = false;
                        break;
                    }
                }
                if (is_star)
                {
                    const unsigned char red[] = {255, 0, 0};
                    stars.draw_circle(x, y, r, red, 1);
                    count++;
                }
            }
        }
    }
    DLOG_F(INFO, "Finished detecting {} stars in image: {}", count, filename);
}

void ImageProcessingPlugin::compress_image(CImg<unsigned char> &img, int compress_ratio)
{
    // 计算压缩后的图像宽度和高度，并创建新图像
    int new_width = img.width() / compress_ratio, new_height = img.height() / compress_ratio;
    CImg<unsigned char> new_img(new_width, new_height, 1, img.spectrum());

    // 对每个新像素点，计算与原图像对应的多个像素的平均值，并输出调试日志
    DLOG_F(INFO, "Compress the image with ratio {}.", compress_ratio);
    cimg_forXY(new_img, x, y)
    {
        int sum_r = 0, sum_g = 0, sum_b = 0, count = 0;
        for (int i = 0; i < compress_ratio; i++)
        {
            for (int j = 0; j < compress_ratio; j++)
            {
                int px = x * compress_ratio + i, py = y * compress_ratio + j;
                if (px < img.width() && py < img.height())
                {
                    sum_r += img(px, py, 0, 0);
                    if (img.spectrum() > 1)
                    {
                        sum_g += img(px, py, 0, 1);
                        sum_b += img(px, py, 0, 2);
                    }
                    count++;
                }
            }
        }
        new_img(x, y, 0, 0) = sum_r / count;
        if (img.spectrum() > 1)
        {
            new_img(x, y, 0, 1) = sum_g / count;
            new_img(x, y, 0, 2) = sum_b / count;
        }
    }

    img = new_img; // 将压缩后的图像覆盖原始图像
}

CImg<unsigned char> ImageProcessingPlugin::read_image(const char *filename)
{
    CImg<unsigned char> img;
    // 检查文件是否存在
    if (std::ifstream(filename))
    {
        // 读取图片
        try
        {
            img.load(filename);
        }
        catch (CImgIOException &e)
        {
            LOG_F(ERROR, "Error reading image file: {}", e.what());
        }
    }
    else
    {
        LOG_F(ERROR, "Error: image file does not exist");
    }
    return img;
}

bool ImageProcessingPlugin::read_color_image(const char *filename, CImg<unsigned char> &image)
{
    image.load(filename);
    if (image.spectrum() == 1)
    {
        return false;
    }
    return true;
}

bool convert_fits_to_cimg(fitsfile *fits_image, CImg<unsigned char> &cimg_image)
{
    long fpixel[3] = {1, 1, 1};
    int status = 0;
    long nelements = cimg_image.width() * cimg_image.height();
    unsigned char *buffer = new unsigned char[nelements];
    fits_read_pix(fits_image, TBYTE, fpixel, nelements, nullptr, buffer, nullptr, &status);

    if (status != 0)
    {
        LOG_F(ERROR, "Failed to read FITS image data: {}", status);
        delete[] buffer;
        return false;
    }

    // 将数据从buffer中拷贝到CImg对象中
    cimg_forXY(cimg_image, x, y)
    {
        cimg_image(x, y) = buffer[y * cimg_image.width() + x];
    }

    delete[] buffer;
    return true;
}

bool ImageProcessingPlugin::read_fits_image(const char *filename, CImg<unsigned char> &image)
{
    fitsfile *fits_image;
    int status = 0;
    fits_open_file(&fits_image, filename, READONLY, &status);
    if (status != 0)
    {
        LOG_F(ERROR, "Failed to open FITS file {}: {}", filename, status);
        return false;
    }

    // 获取图像大小和通道数
    int naxis;
    long naxes[3];
    fits_get_img_dim(fits_image, &naxis, &status);
    fits_get_img_size(fits_image, 3, naxes, &status);

    if (naxis == 3 && naxes[2] != 1)
    {
        fits_close_file(fits_image, &status);
        return false;
    }

    // 转化为CImg格式
    image.assign(naxes[0], naxes[1], 1, 1);
    // convert_fits_to_cimg(fits_image, image);

    fits_close_file(fits_image, &status);
    return true;
}

bool ImageProcessingPlugin::save_image(const CImg<unsigned char> &image, const char *filename)
{
    try
    {
        image.save(filename);
        return true;
    }
    catch (CImgIOException &)
    {
        return false;
    }
}

void ImageProcessingPlugin::overlay_image(CImg<unsigned char> &img1, const CImg<unsigned char> &img2)
{
    int width = std::min(img1.width(), img2.width());
    int height = std::min(img1.height(), img2.height());

    cimg_forXY(img1, x, y)
    {
        if (x < width && y < height)
        {
            img1(x, y) = static_cast<unsigned char>(std::round((static_cast<double>(img1(x, y)) + static_cast<double>(img2(x, y))) / 2));
        }
    }
}

void ImageProcessingPlugin::computeHistogram(const char *filename, int *hist, int nbins)
{
    // 加载图像
    CImg<unsigned char> img(filename);

    // 清空直方图
    std::memset(hist, 0, sizeof(int) * nbins);

    // 计算直方图
    cimg_for(img, ptr, unsigned char)
    {
        hist[*ptr]++;
    }
}

// 显示直方图
void ImageProcessingPlugin::displayHistogram(const int *hist, int nbins)
{
    const int hist_w = 512, hist_h = 400;
    CImg<unsigned char> hist_img(hist_w, hist_h, 1, 3, 255);
    const int hist_max = *std::max_element(hist, hist + nbins);
    for (int i = 0; i < nbins; i++)
    {
        const int x0 = i * hist_w / nbins, x1 = (i + 1) * hist_w / nbins;
        const int y = hist[i] * (hist_h - 1) / hist_max;
    }
    hist_img.display();
}