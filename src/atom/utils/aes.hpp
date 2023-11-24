/*
 * aes.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-11-24

Description: Simple implementation of AES encryption

**************************************************/

#pragma once

#include <string>

/**
 * @brief 使用AES算法对输入的明文进行加密。
 * 
 * @param plaintext 明文数据
 * @param key 加密密钥
 * @return 加密后的密文数据
 */
std::string encryptAES(const std::string& plaintext, const std::string& key);

/**
 * @brief 使用AES算法对输入的密文进行解密。
 * 
 * @param ciphertext 密文数据
 * @param key 解密密钥
 * @return 解密后的明文数据
 */
std::string decryptAES(const std::string& ciphertext, const std::string& key);

/**
 * @brief 使用Zlib库对输入的数据进行压缩。
 * 
 * @param data 待压缩的数据
 * @return 压缩后的数据
 */
std::string compress(const std::string& data);

/**
 * @brief 使用Zlib库对输入的数据进行解压。
 * 
 * @param data 待解压的数据
 * @return 解压后的数据
 */
std::string decompress(const std::string& data);
