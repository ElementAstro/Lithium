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

Date: 2023-7-13

Description: Base64

**************************************************/

#pragma once

#include <stddef.h>

/**
 * \brief 将字节数组转换为base64编码的字符串。
 *
 * \param out 输出的base64编码字符串。输出缓冲区大小必须至少为 (4 * inlen / 3 + 4) 字节。
 * \param in 输入的二进制缓冲区。
 * \param inlen 要转换的字节数。
 * \param outlen 输出缓冲区的大小。
 * \return 成功返回0，失败返回-1。
 */
size_t to64frombits_s(unsigned char *out, const unsigned char *in, int inlen, size_t outlen);

/**
 * \brief 将base64编码的字符串转换为字节数组。
 *
 * \param out 输出的二进制缓冲区。输出缓冲区的大小必须至少为 (3 * size_of_in_buffer / 4) 字节。
 * \param in 输入的base64编码字符串。
 * \param inlen base64编码字符串的长度。
 * \return 成功返回0，失败返回-1。
 */
int from64tobits(char *out, const char *in);

/**
 * \brief 快速将base64编码的字符串转换为字节数组。
 *
 * \param out 输出的二进制缓冲区。输出缓冲区的大小必须至少为 (3 * size_of_in_buffer / 4) 字节。
 * \param in 输入的base64编码字符串。
 * \param inlen base64编码字符串的长度。
 * \return 成功返回0，失败返回-1。
 */
int from64tobits_fast(char *out, const char *in, int inlen);
