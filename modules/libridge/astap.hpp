/*
 * astap.hpp
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

Date: 2023-4-4

Description: Astap Solver Interface

**************************************************/

#pragma once

#include <map>
#include <string>

namespace Lithium::API::ASTAP
{
    /**
     * @brief 使用 ASTAP 进行图像匹配，并返回匹配结果。如果匹配成功，返回包含星表数据的映射；否则返回一个错误消息。
     *
     * @param ra 图像所在区域的赤经（度）。如果不需要指定
     * @param dec 图像所在区域的赤纬（度）。如果不需要指定
     * @param fov 图像视场（度）。如果不需要指定
     * @param timeout 执行命令的超时时间（秒数）。默认值为30秒。
     * @param update 是否在执行匹配后更新图片。默认为true。
     * @param image 需要匹配的图像文件路径。该参数必须提供。
     * @return 如果匹配成功，返回包含星表数据的映射；否则返回一个错误消息。
     * @brief run_astap function
     *
     * This function uses the ASTAP algorithm to perform image matching and returns the matching results.
     * If the matching is successful, it returns a map containing the star catalog data; otherwise, it returns an error message.
     *
     * @param ra The right ascension of the image region (degrees). If not specified
     * @param dec The declination of the image region (degrees). If not specified
     * @param fov The field of view of the image (degrees). If not specified
     * @param timeout The timeout for executing the command (in seconds). The default value is 30 seconds.
     * @param update Whether to update the image after matching. The default value is true.
     * @param image The path of the image file to be matched. This parameter is required.
     * @return If the matching is successful, it returns a map containing the star catalog data; otherwise, it returns an error message.
     *
     * */
    std::map<std::string, std::string> run_astap(double ra = 0.0, double dec = 0.0, double fov = 0.0, int timeout = 30, bool update = true, std::string image = "");
} // namespace Lithium::API::ASTAP
