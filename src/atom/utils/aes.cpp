/*
 * aes.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-11-24

Description: Simple implementation of AES encryption

**************************************************/

#include "aes.hpp"

#include <string_view>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <zlib.h>

const int AES_BLOCK_SIZE = 16;

namespace Atom::Utils
{
    std::string encryptAES(const std::string &plaintext, const std::string &key)
    {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), nullptr);

        int c_len = plaintext.length() + AES_BLOCK_SIZE;
        unsigned char *ciphertext = new unsigned char[c_len];

        int len;
        EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<const unsigned char *>(plaintext.c_str()), plaintext.length());
        int ciphertext_len = len;

        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        ciphertext_len += len;

        std::string result(reinterpret_cast<char *>(ciphertext), ciphertext_len);

        delete[] ciphertext;
        EVP_CIPHER_CTX_free(ctx);

        return result;
    }

    std::string decryptAES(const std::string &ciphertext, const std::string &key)
    {
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr, reinterpret_cast<const unsigned char *>(key.c_str()), nullptr);

        int p_len = ciphertext.length() + AES_BLOCK_SIZE;
        unsigned char *plaintext = new unsigned char[p_len];

        int len;
        EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<const unsigned char *>(ciphertext.c_str()), ciphertext.length());
        int plaintext_len = len;

        EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
        plaintext_len += len;

        std::string result(reinterpret_cast<char *>(plaintext), plaintext_len);

        delete[] plaintext;
        EVP_CIPHER_CTX_free(ctx);

        return result;
    }

    std::string compress(const std::string &data)
    {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
        {
            return "";
        }

        zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
        zs.avail_in = data.size();

        int ret;
        char outbuffer[32768];
        std::string compressed;

        do
        {
            zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);
            if (ret == Z_STREAM_END || zs.avail_out != sizeof(outbuffer))
            {
                compressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        return compressed;
    }

    std::string decompress(const std::string &data)
    {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        if (inflateInit(&zs) != Z_OK)
        {
            return "";
        }

        zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
        zs.avail_in = data.size();

        int ret;
        char outbuffer[32768];
        std::string decompressed;

        do
        {
            zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);
            if (ret == Z_STREAM_END || zs.avail_out != sizeof(outbuffer))
            {
                decompressed.append(outbuffer, sizeof(outbuffer) - zs.avail_out);
            }
        } while (ret == Z_OK);

        inflateEnd(&zs);

        return decompressed;
    }
}

