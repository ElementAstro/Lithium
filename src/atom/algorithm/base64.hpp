/*
 * base64.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Base64

**************************************************/

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace Atom::Utils {

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

/**
 * @brief Base64编码函数
 *
 * @param bytes_to_encode 待编码数据
 * @return std::string 编码后的字符串
 */
std::string base64EncodeEnhance(const std::vector<uint8_t> &bytes_to_encode);

/**
 * @brief Base64解码函数
 *
 * @param encoded_string 待解码字符串
 * @return std::vector<unsigned char> 解码后的数据
 */
std::vector<uint8_t> base64DecodeEnhance(const std::string &encoded_string);

}  // namespace Atom::Utils
