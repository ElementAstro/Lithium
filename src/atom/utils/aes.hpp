/*
 * aes.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-24

Description: Simple implementation of AES encryption

**************************************************/

#ifndef ATOM_UTILS_AES_HPP
#define ATOM_UTILS_AES_HPP

#include <string>

namespace atom::utils {
/**
 * @brief 使用AES算法对输入的明文进行加密。
 *
 * @param plaintext 明文数据
 * @param key 加密密钥
 * @return 加密后的密文数据
 */
[[nodiscard]] std::string encryptAES(std::string_view plaintext,
                                     std::string_view key);

/**
 * @brief 使用AES算法对输入的密文进行解密。
 *
 * @param ciphertext 密文数据
 * @param key 解密密钥
 * @return 解密后的明文数据
 */
[[nodiscard]] std::string decryptAES(std::string_view ciphertext,
                                     std::string_view key);

/**
 * @brief 使用Zlib库对输入的数据进行压缩。
 *
 * @param data 待压缩的数据
 * @return 压缩后的数据
 */
[[nodiscard]] std::string compress(std::string_view data);

/**
 * @brief 使用Zlib库对输入的数据进行解压。
 *
 * @param data 待解压的数据
 * @return 解压后的数据
 */
[[nodiscard]] std::string decompress(std::string_view data);

/**
 * @brief 计算文件的SHA-256哈希值。
 *
 * @param filename 文件名
 * @return 文件的SHA-256哈希值
 */
[[nodiscard]] std::string calculateSha256(std::string_view filename);
}  // namespace atom::utils

#endif
