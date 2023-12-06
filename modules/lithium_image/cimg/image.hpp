/*
 * image.hpp
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

#pragma once

#include "core/plugin/plugin.hpp"
#include "cimg/CImg.h"
#include <fitsio.h>

using namespace cimg_library;

class ImageProcessingPlugin : public Plugin
{
public:
    ImageProcessingPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);

    void Blur(const json &params);

    void Rotate(const json &params);

    void Crop(const json &params);

    void Sharpen(const json &params);

    void WhiteBalance(const json &params);

    void Resize(const json &params);

    void ImageToBase64(const json &params);

    void Base64ToImage(const json &params);

private:
    double calc_hfd(const CImg<unsigned char> &img, int outer_diameter);

    void calc_dark_noise(const CImg<unsigned char> &dark_img, float &average_dark, float &sigma_dark, float &sigma_readout);

    void detect_stars(const char *filename, int threshold, int max_radius);

    void compress_image(CImg<unsigned char> &img, int compress_ratio);
    CImg<unsigned char> read_image(const char *filename);

    bool read_color_image(const char *filename, CImg<unsigned char> &image);

    bool convert_fits_to_cimg(fitsfile *fits_image, CImg<unsigned char> &cimg_image);

    bool read_fits_image(const char *filename, CImg<unsigned char> &image);

    bool save_image(const CImg<unsigned char> &image, const char *filename);

    void overlay_image(CImg<unsigned char> &img1, const CImg<unsigned char> &img2);

    void computeHistogram(const char *filename, int *hist, int nbins);

    void displayHistogram(const int *hist, int nbins);

private:
    mutable std::unordered_map<std::string, CImg<unsigned char>> imageCache;
};