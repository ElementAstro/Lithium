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
    EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr,
                       reinterpret_cast<const unsigned char *>(key.data()),
                       nullptr);

    unsigned long cLen = plaintext.length() + AES_BLOCK_SIZE;
    auto *ciphertext = new unsigned char[cLen];

    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len,
                      reinterpret_cast<const unsigned char *>(plaintext.data()),
                      plaintext.length());
    int ciphertextLen = len;

    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertextLen += len;

    std::string result(reinterpret_cast<char *>(ciphertext), ciphertextLen);

    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string decryptAES(std::string_view ciphertext, std::string_view key) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr,
                       reinterpret_cast<const unsigned char *>(key.data()),
                       nullptr);

    int pLen = ciphertext.length() + AES_BLOCK_SIZE;
    auto *plaintext = new unsigned char[pLen];

    int len;
    EVP_DecryptUpdate(
        ctx, plaintext, &len,
        reinterpret_cast<const unsigned char *>(ciphertext.data()),
        ciphertext.length());
    int plaintextLen = len;

    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintextLen += len;

    std::string result(reinterpret_cast<char *>(plaintext), plaintextLen);

    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string compress(std::string_view data) {
    z_stream zstream;
    memset(&zstream, 0, sizeof(zstream));

    if (deflateInit(&zstream, Z_BEST_COMPRESSION) != Z_OK) {
        return "";
    }

    zstream.next_in =
        reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    zstream.avail_in = data.size();

    int ret;
    constexpr size_t BUFFER_SIZE = 32768;
    std::array<char, BUFFER_SIZE> outbuffer{};
    std::string compressed;

    do {
        zstream.next_out = reinterpret_cast<Bytef *>(outbuffer.data());
        zstream.avail_out = outbuffer.size();

        ret = deflate(&zstream, Z_FINISH);
        if (ret == Z_STREAM_END || zstream.avail_out != outbuffer.size()) {
            compressed.append(outbuffer.data(),
                              outbuffer.size() - zstream.avail_out);
        }
    } while (ret == Z_OK);

    deflateEnd(&zstream);

    return compressed;
}

std::string decompress(std::string_view data) {
    z_stream zstream;
    memset(&zstream, 0, sizeof(zstream));

    if (inflateInit(&zstream) != Z_OK) {
        return "";
    }

    zstream.next_in =
        reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    zstream.avail_in = data.size();

    int ret;
    constexpr size_t BUFFER_SIZE = 32768;
    std::array<char, BUFFER_SIZE> outbuffer{};
    std::string decompressed;

    do {
        zstream.next_out = reinterpret_cast<Bytef *>(outbuffer.data());
        zstream.avail_out = outbuffer.size();

        ret = inflate(&zstream, 0);
        if (ret == Z_STREAM_END || zstream.avail_out != outbuffer.size()) {
            decompressed.append(outbuffer.data(),
                                outbuffer.size() - zstream.avail_out);
        }
    } while (ret == Z_OK);

    inflateEnd(&zstream);

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
    if (mdctx == nullptr) {
        return "";
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    constexpr size_t BUFFER_SIZE = 1024;
    std::array<char, BUFFER_SIZE> buffer{};
    while (file.read(buffer.data(), buffer.size())) {
        if (EVP_DigestUpdate(mdctx, buffer.data(), buffer.size()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }

    if (file.gcount() > 0) {
        if (EVP_DigestUpdate(mdctx, buffer.data(), file.gcount()) != 1) {
            EVP_MD_CTX_free(mdctx);
            return "";
        }
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> hash{};
    unsigned int hashLen = 0;
    if (EVP_DigestFinal_ex(mdctx, hash.data(), &hashLen) != 1) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    EVP_MD_CTX_free(mdctx);

    // Convert to hexadecimal string
    std::ostringstream sha256Val;
    for (unsigned int i = 0; i < hashLen; ++i) {
        sha256Val << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(hash[i]);
    }

    return sha256Val.str();
}
}  // namespace atom::utils
