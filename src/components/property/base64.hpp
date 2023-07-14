/*
 * base64.hpp
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

Date: 2023-4-5

Description: Base64

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace Lithium::Base64
{

    /**
     * @brief Base64编码函数
     *
     * @param bytes_to_encode 待编码数据
     * @return std::string 编码后的字符串
     */
    std::string base64Encode(const std::vector<unsigned char> &bytes_to_encode);

    /**
     * @brief Base64解码函数
     *
     * @param encoded_string 待解码字符串
     * @return std::vector<unsigned char> 解码后的数据
     */
    std::vector<unsigned char> base64Decode(const std::string &encoded_string);

}
