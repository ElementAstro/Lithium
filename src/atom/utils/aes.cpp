/*
 * aes.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-24

Description: Simple implementation of AES encryption

**************************************************/

#include "aes.hpp"

#include <cstring>
#include <fstream>
#include <string_view>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <zlib.h>

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"

const int AES_BLOCK_SIZE = 16;

namespace atom::utils {
std::string encryptAES(std::string_view plaintext, std::string_view key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                       reinterpret_cast<const unsigned char *>(key.data()),
                       nullptr);

    int c_len = plaintext.length() + AES_BLOCK_SIZE;
    unsigned char *ciphertext = new unsigned char[c_len];

    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len,
                      reinterpret_cast<const unsigned char *>(plaintext.data()),
                      plaintext.length());
    int ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    std::string result(reinterpret_cast<char *>(ciphertext), ciphertext_len);

    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string decryptAES(std::string_view ciphertext, std::string_view key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                       reinterpret_cast<const unsigned char *>(key.data()),
                       nullptr);

    int p_len = ciphertext.length() + AES_BLOCK_SIZE;
    unsigned char *plaintext = new unsigned char[p_len];

    int len;
    EVP_DecryptUpdate(
        ctx, plaintext, &len,
        reinterpret_cast<const unsigned char *>(ciphertext.data()),
        ciphertext.length());
    int plaintext_len = len;

    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintext_len += len;

    std::string result(reinterpret_cast<char *>(plaintext), plaintext_len);

    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string compress(std::string_view data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        return "";
    }

    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string compressed;

    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);
        if (ret == Z_STREAM_END || zs.avail_out != sizeof(outbuffer)) {
            compressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    return compressed;
}

std::string decompress(std::string_view data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK) {
        return "";
    }

    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string decompressed;

    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);
        if (ret == Z_STREAM_END || zs.avail_out != sizeof(outbuffer)) {
            decompressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    return decompressed;
}

std::string calculateSha256(std::string_view filename) {
    if (!atom::io::isFileExists(std::string(filename))) {
        LOG_F(ERROR, "File not exist: {}", filename);
        return "";
    }
    std::ifstream file(filename.data(), std::ios::binary);
    if (!file || !file.good()) {
        return "";
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(mdctx, buffer, sizeof(buffer));
    }

    if (file.gcount() > 0) {
        EVP_DigestUpdate(mdctx, buffer, file.gcount());
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    // 转换为十六进制字符串
    std::string sha256_val;
    for (unsigned int i = 0; i < hash_len; ++i) {
        char hex_str[3];
        sprintf(hex_str, "%02x", hash[i]);
        sha256_val += hex_str;
    }

    return sha256_val;
}
}  // namespace atom::utils
