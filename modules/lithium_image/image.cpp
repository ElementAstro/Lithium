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

Date: 2023-4-6

Description: Image Processing

**************************************************/

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <cmath>
#include <spdlog/spdlog.h>
#include <fitsio.h>

#include "cimg/CImg.h"

#include "image.hpp"

using namespace cimg_library;

namespace Lithium::Image
{

    CImg<unsigned char> read_image(const char *filename)
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
                spdlog::error("Error reading image file: {}", e.what());
            }
        }
        else
        {
            spdlog::error("Error: image file does not exist");
        }
        return img;
    }

    /**
     * @brief 读取彩色图像，如果是灰度图像则不进行处理并返回 false。
     *
     * @param filename 图像文件名。
     * @param image 输出参数，用于存储读入的图像。
     * @return true 读入成功且为彩色图像；false 读入成功但为灰度图像。
     */
    bool read_color_image(const char *filename, CImg<unsigned char> &image)
    {
        image.load(filename);
        if (image.spectrum() == 1)
        {
            return false;
        }
        return true;
    }

    /**
     * @brief 将FITS格式图像转化为 CImg 格式图像。
     *
     * @param fits_image 要转化的FITS格式图像。
     * @param cimg_image 输出参数，用于存储转化后的CImg格式图像。
     * @return true 转化成功；false 转化失败。
     */
    bool convert_fits_to_cimg(fitsfile *fits_image, CImg<unsigned char> &cimg_image)
    {
        long fpixel[3] = {1, 1, 1};
        int status = 0;
        long nelements = cimg_image.width() * cimg_image.height();
        unsigned char *buffer = new unsigned char[nelements];
        fits_read_pix(fits_image, TBYTE, fpixel, nelements, nullptr, buffer, nullptr, &status);

        if (status != 0)
        {
            spdlog::error("Failed to read FITS image data: {}", status);
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

    /**
     * @brief 读取FITS格式图像，如果读入的图像是彩色图像则返回 false。
     *
     * @param filename 图像文件名。
     * @param image 输出参数，用于存储读入的图像。
     * @return true 读入成功且为灰度图像；false 读入成功但为彩色图像。
     */
    bool read_fits_image(const char *filename, CImg<unsigned char> &image)
    {
        fitsfile *fits_image;
        int status = 0;
        fits_open_file(&fits_image, filename, READONLY, &status);
        if (status != 0)
        {
            spdlog::error("Failed to open FITS file {}: {}", filename, status);
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
        convert_fits_to_cimg(fits_image, image);

        fits_close_file(fits_image, &status);
        return true;
    }

    /**
     * @brief 保存图像到文件。
     *
     * @param image 要保存的图像。
     * @param filename 文件名。
     * @return true 保存成功；false 保存失败。
     */
    bool save_image(const CImg<unsigned char> &image, const char *filename)
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

    /**
     * @brief 裁剪图像
     *
     * @param img 输入图像
     * @param x 裁剪区域左上角x坐标
     * @param y 裁剪区域左上角y坐标
     * @param w 裁剪区域宽度
     * @param h 裁剪区域高度
     */
    void crop_image(CImg<unsigned char> &img, int x, int y, int w, int h)
    {
        // 调用CImg库提供的crop()函数进行图像裁剪，并输出调试日志
        DLOG_F(INFO,"Crop the image to ({}, {}), width = {}, height = {}.", x, y, w, h);
        img.crop(x, y, x + w - 1, y + h - 1);
    }

    /**
     * @brief 旋转图像
     *
     * @param img 输入图像
     * @param angle 旋转角度（单位：度）
     */
    void rotate_image(CImg<unsigned char> &img, float angle)
    {
        // 调用CImg库提供的rotate()函数进行图像旋转，并输出调试日志
        DLOG_F(INFO,"Rotate the image by {} degrees.", angle);
        img.rotate(angle);
    }

    /**
     * @brief 图像翻转函数
     *
     * @param img CImg类的引用，表示要翻转的图像
     * @param direction 整数类型，表示翻转方向。0表示水平翻转，1表示垂直翻转
     */
    void flip(CImg<unsigned char> &img, int direction)
    {
        // 添加spdlog的debug输出
        DLOG_F(INFO,"Flipping image...");

        if (direction == 0)
        { // 水平翻转
            cimg_forXY(img, x, y)
            {
                const int new_x = img.width() - 1 - x;
                std::swap(img(x, y, 0), img(new_x, y, 0));
            }
        }
        else if (direction == 1)
        { // 垂直翻转
            cimg_forXY(img, x, y)
            {
                const int new_y = img.height() - 1 - y;
                std::swap(img(x, y, 0), img(x, new_y, 0));
            }
        }

        // 添加spdlog的debug输出
        DLOG_F(INFO,"Image flipped.");
    }

    /**
     * @brief 计算图像直方图
     *
     * @param img 输入图像
     * @return std::vector<int> 图像直方图，存储在vector<int>中
     */
    std::vector<int> compute_histogram(CImg<unsigned char> &img)
    {
        // 使用vector<int>存储图像直方图，并输出调试日志
        DLOG_F(INFO,"Compute the histogram of the image.");
        std::vector<int> hist(256, 0);
        cimg_forXY(img, x, y)
        {
            int val = (int)img(x, y, 0, 0); // 取出像素值
            hist[val]++;
        }
        return hist;
    }

    /**
     * @brief 压缩图像
     *
     * @param img 输入图像
     * @param compress_ratio 压缩比例
     */
    void compress_image(CImg<unsigned char> &img, int compress_ratio)
    {
        // 计算压缩后的图像宽度和高度，并创建新图像
        int new_width = img.width() / compress_ratio, new_height = img.height() / compress_ratio;
        CImg<unsigned char> new_img(new_width, new_height, 1, img.spectrum());

        // 对每个新像素点，计算与原图像对应的多个像素的平均值，并输出调试日志
        DLOG_F(INFO,"Compress the image with ratio {}.", compress_ratio);
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

    /**
     * @brief 高斯滤波
     *
     * @param image 输入图像
     * @return CImg<unsigned char> 经过高斯滤波后的输出图像
     */
    CImg<unsigned char> gaussian_filter(const CImg<unsigned char> &image)
    {
        float kernel[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
        CImg<float> filter(kernel, 3, 3);
        DLOG_F(INFO,"Apply Gaussian filter to the image.");
        return image.get_convolve(filter);
    }

    /**
     * @brief 均值滤波
     *
     * @param image 输入图像
     * @return CImg<unsigned char> 经过均值滤波后的输出图像
     */
    CImg<unsigned char> mean_filter(const CImg<unsigned char> &image)
    {
        float kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
        CImg<float> filter(kernel, 3, 3);
        DLOG_F(INFO,"Apply mean filter to the image.");
        return image.get_convolve(filter);
    }

    /**
     * @brief 锐化
     *
     * @param image 输入图像
     * @return CImg<unsigned char> 经过锐化操作后的输出图像
     */
    CImg<unsigned char> sharpen(const CImg<unsigned char> &image)
    {
        float kernel[9] = {-1, -1, -1, -1, 9, -1, -1, -1, -1};
        CImg<float> filter(kernel, 3, 3);
        DLOG_F(INFO,"Apply sharpen filter to the image.");
        return image.get_convolve(filter);
    }

    /**
     * @brief 对图像进行拉伸
     *
     * @param img 原始图像
     * @return CImg<unsigned char> 拉伸后的图像
     */
    CImg<unsigned char> stretch_image(const CImg<unsigned char> &img)
    {
        return img.get_normalize(0, 255);
    }

    /**
     * @brief 调整图像的亮度
     *
     * @param img 原始图像
     * @param ratio 亮度调整比例
     * @return CImg<unsigned char> 调整后的图像
     */
    CImg<unsigned char> brighten_image(const CImg<unsigned char> &img, float ratio)
    {
        return img * ratio;
    }

    /**
     * @brief 检测图像中的星点
     *
     * @param filename 图像文件名
     * @param threshold 二值化阈值
     * @param max_radius 最大圆半径
     */
    void detect_stars(const char *filename, int threshold, int max_radius)
    {
        // 读取图像并记录日志
        DLOG_F(INFO,"Loading image: {}", filename);
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
        DLOG_F(INFO,"Finished detecting {} stars in image: {}", count, filename);
    }

    /**
     * @brief 判断图像位数
     *
     * @param img 输入图像
     */
    int bit_depth(const CImg<unsigned char> &img)
    {
        int depth = img.depth();
        if (depth == 1)
        {
            DLOG_F(INFO,"The bit depth of the image is: 1 bit");
        }
        else if (depth == 3)
        {
            DLOG_F(INFO,"The bit depth of the image is: 24 bits");
        }
        else
        {
            DLOG_F(INFO,"The bit depth of the image is: {} bits", depth * 8);
        }
        return depth;
    }

    /**
     * @brief 计算图像曝光时间
     *
     * @param img 输入图像
     * @param iso ISO感光度
     * @param aperture 光圈值
     * @param shutter_speed 快门速度
     */
    float calc_exposure_time(const CImg<unsigned char> &img, const int iso, const float aperture, const float shutter_speed)
    {
        // 计算光量总量
        float total_light = 0;
        cimg_forXY(img, x, y)
        {
            total_light += img(x, y, 0) + img(x, y, 1) + img(x, y, 2);
        }

        // 计算曝光时间
        float exposure_time = 100.0f * iso * aperture * aperture / (shutter_speed * total_light);
        DLOG_F(INFO,"The exposure time of the astronomy camera is: {}s", exposure_time);
        return exposure_time;
    }

    /**
     * @brief 计算天文相机曝光时间
     *
     * @param gain 增益倍数
     * @param t 曝光时间
     * @param dark_noise 暗电流噪声
     * @param read_noise 读出噪声
     */
    float calc_astro_exposure_time(const float gain, const float t, const float dark_noise, const float read_noise)
    {
        // 计算曝光时间
        float exposure_time = (gain * t) / (dark_noise * dark_noise - read_noise * read_noise);
        DLOG_F(INFO,"The exposure time of the astronomy camera is: {}s", exposure_time);
        return exposure_time;
    }

    /**
     * @brief 计算暗电流噪声和读出噪声
     *
     * @param dark_img 暗场图像
     * @param average_dark 均值（暗电流）
     * @param sigma_dark 标准差（暗电流）
     * @param sigma_readout 标准差（读出噪声）
     */
    void calc_dark_noise(const CImg<unsigned char> &dark_img, float &average_dark, float &sigma_dark, float &sigma_readout)
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

    /**
     * @brief 计算图像的 HFD
     *
     * @param img 待计算的图像
     * @param outer_diameter 外径直径
     * @return double 返回 HFD 值
     */
    double calc_hfd(const CImg<unsigned char> &img, int outer_diameter)
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

    /**
     * @brief 计算图像的均值、方差、峰值信噪比等图像质量参数
     *
     * @param img 待检测的图像
     * @return std::tuple<double, double, double> 返回一个元组，包含图像的均值、方差和峰值信噪比
     */
    std::tuple<double, double, double, double> image_quality_measures(const CImg<unsigned char> &img)
    {
        double mean_value = img.mean();
        double variance = img.variance();
        double max_value = img.max();
        double noise_power = variance / (max_value * max_value);
        double peak_signal_noise_ratio = 10 * log10(max_value * max_value / variance);
        return std::make_tuple(mean_value, variance, peak_signal_noise_ratio, noise_power);
    }

    /**
     * @brief 计算图像的均值
     *
     * @param img 待计算的图像
     * @return double 返回图像的均值
     */
    double calc_mean(const CImg<unsigned char> &img)
    {
        return img.mean();
    }

    /**
     * @brief 对图像进行校准
     *
     * @param img 待校准的图像
     * @param threshold 校准阈值
     * @return void
     */
    void calibrate_image(CImg<unsigned char> &img, double threshold)
    {
        double mean = calc_mean(img);

        cimg_forXY(img, x, y)
        {
            if (img(x, y) <= threshold)
            {
                img(x, y) = 0;
            }
            else
            {
                img(x, y) = static_cast<unsigned char>(std::round((img(x, y) - threshold) * 255.0 / (mean - threshold)));
            }
        }
    }

    /**
     * @brief 计算图像相似度
     *
     * @param img1 图像1
     * @param img2 图像2
     * @return double 返回两个图像之间的相似度
     */
    double calc_similarity(const CImg<unsigned char> &img1, const CImg<unsigned char> &img2)
    {
        int width = std::min(img1.width(), img2.width());
        int height = std::min(img1.height(), img2.height());

        double sum_diff = 0;
        cimg_forXY(img1, x, y)
        {
            if (x < width && y < height)
            {
                sum_diff += std::abs(static_cast<double>(img1(x, y)) - static_cast<double>(img2(x, y)));
            }
        }

        return 1 - sum_diff / (255.0 * width * height);
    }

    /**
     * @brief 将图像2叠加到图像1上
     *
     * @param img1 图像1
     * @param img2 图像2
     * @return void
     */
    void overlay_image(CImg<unsigned char> &img1, const CImg<unsigned char> &img2)
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

    /**
     * @brief 给图像添加椒盐噪声
     *
     * @param image 原始图像
     * @param threshold 噪声密度
     * @return CImg<double> 添加噪声后的图像
     */
    CImg<double> add_salt_pepper_noise(CImg<double> image, double threshold)
    {
        CImg<double> output(image.width(), image.height(), 1, image.spectrum(), 0);
        double thres = 1 - threshold;
        cimg_forXY(image, x, y)
        {
            double randomnum = ((double)rand() / RAND_MAX);
            if (randomnum < threshold)
            {
                output(x, y) = 0;
            }
            else if (randomnum > thres)
            {
                output(x, y) = 255;
            }
            else
            {
                output(x, y) = image(x, y);
            }
        }
        return output;
    }

    /**
     * @brief 给图像添加高斯噪声
     *
     * @param image 原始图像
     * @param mean 均值
     * @param var 方差
     * @return CImg<double> 添加噪声后的图像
     */
    CImg<double> add_gaussian_noise(CImg<double> image, double mean, double var)
    {
        CImg<double> output(image.width(), image.height(), 1, image.spectrum(), 0);
        CImg<double> noise(image.width(), image.height(), 1, image.spectrum(), 0);
        cimg_forXY(image, x, y)
        {
            noise(x, y) = ((double)rand() / RAND_MAX) * var + mean;
            output(x, y) = image(x, y) + noise(x, y);
        }
        return output;
    }

    /**
     * @brief 将彩色图像按照颜色通道拆分为不同的图片。
     *
     * @param image 要拆分的彩色图像。
     * @param r 输出参数，用于存储红色通道。
     * @param g 输出参数，用于存储绿色通道。
     * @param b 输出参数，用于存储蓝色通道。
     */
    void split_color_image(const CImg<unsigned char> &image, CImg<unsigned char> &r, CImg<unsigned char> &g, CImg<unsigned char> &b)
    {
        r = image.get_channel(0);
        g = image.get_channel(1);
        b = image.get_channel(2);
    }

}

/*
int main() {
    CImg<unsigned char> img = read_image("test.jpg");

    // 裁剪图像
    crop_image(img, 100, 100, 200, 200);

    // 旋转图像
    rotate_image(img, 45.0f);

    // 计算直方图
    std::vector<int> hist = compute_histogram(img);

    // 压缩图像
    compress_image(img, 4);

    // 读取图像
    CImg<unsigned char> image("example.jpg");

    // 进行滤波和锐化操作
    CImg<unsigned char> result_gaussian = gaussian_filter(image), result_mean = mean_filter(image), result_sharpen = sharpen(image);

    // 显示结果
    CImgDisplay original_disp(image, "Original Image"), gaussian_disp(result_gaussian, "Gaussian Filtered Image"), mean_disp(result_mean, "Mean Filtered Image"), sharpen_disp(result_sharpen, "Sharpened Image");
    while (!original_disp.is_closed() || !gaussian_disp.is_closed() || !mean_disp.is_closed() || !sharpen_disp.is_closed()) {
        original_disp.wait();
    }

    // 设置ISO感光度、光圈值和快门速度
    int iso = 100;
    float aperture = 2.8f;
    float shutter_speed = 1.0f / 250;

    // 调用函数计算曝光时间
    calc_exposure_time(img, iso, aperture, shutter_speed);

    // 计算暗电流噪声和读出噪声
    float average_dark, sigma_dark, sigma_readout;
    calc_dark_noise(img, average_dark, sigma_dark, sigma_readout);

    std::cout << "The average dark value is: " << average_dark << std::endl;
    std::cout << "The dark noise is: " << sigma_dark << std::endl;
    std::cout << "The readout noise is: " << sigma_readout << std::endl;

    double hfd = calc_hfd(img, 60);
    std::cout << "HFD: " << hfd << std::endl;

    // 计算图像质量参数
    std::tuple<double, double, double, double> quality_measures = image_quality_measures(img);
    double mean_value = std::get<0>(quality_measures);
    double variance = std::get<1>(quality_measures);
    double peak_signal_noise_ratio = std::get<2>(quality_measures);

    // 显示结果
    std::cout << "Mean value: " << mean_value << std::endl;
    std::cout << "Variance: " << variance << std::endl;
    std::cout << "Peak signal-to-noise ratio: " << peak_signal_noise_ratio << std::endl;

    CImg<unsigned char> img_new;

    // 进行图像校准和比对
    calibrate_image(img, 50);

    while (true) {
        // 加载新的图像
        img_new.load("image2.bmp");

        // 进行图像校准和比对
        calibrate_image(img_new, 50);

        double similarity = calc_similarity(img, img_new);

        if (similarity >= 0.9) {
            // 将新图像叠加到原图像上
            overlay_image(img, img_new);

            // 显示叠加后的图像
            CImgDisplay disp(img, "Overlayed image");
            while (!disp.is_closed()) {
                disp.wait();
            }
        }
        else {
            std::cout << "Image not matched. Similarity: " << similarity << std::endl;
        }
    }

    CImg<unsigned char> r, g, b;
    split_color_image(img, r, g, b);
    save_image(r, "red_channel.jpg");
    save_image(g, "green_channel.jpg");
    save_image(b, "blue_channel.jpg");

    return 0;
}

*/
