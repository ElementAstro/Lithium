/*
 * lithium_image_main.cpp
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

Description: Image processing plugin main

**************************************************/

#include "config.h"

#if ENABLE_CIMG
#include "cimg/image.hpp"
#endif
#if ENABLE_OPENCV
#include "opencv/image.hpp"
#endif
#include <memory>

std::shared_ptr<ImageProcessingPlugin> GetInstance()
{
    return std::make_shared<ImageProcessingPlugin>("lithium_image", "1.0.0", "Max Qian", "Image processing plugin");
}

json GetInfo()
{
    json config;
    config["name"] = "lithium_image";
    config["version"] = "1.0.0";
    config["description"] = "Image processing plugin";
    config["author"] = "Max Qian";
    config["email"] = "astro_air@126.com";
    config["url"] = "lightapt.com";
    config["license"] = "GPLv3";
    config["copyright"] = "2023 Max Qian. All rights reserved";
    return config;
}