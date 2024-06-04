#include "atom/utils/aes.hpp"
#include <gtest/gtest.h>
#include <fstream>

TEST(AESTest, EncryptAndDecrypt) {
    std::string plaintext = "Hello, World!";
    std::string key = "supersecretkey";

    std::string ciphertext = atom::utils::encryptAES(plaintext, key);
    EXPECT_NE(ciphertext, plaintext);  // 加密后的密文应与原始明文不同

    std::string decryptedtext = atom::utils::decryptAES(ciphertext, key);
    EXPECT_EQ(decryptedtext, plaintext);
}

// Tests for compress and decompress
TEST(CompressionTest, CompressAndDecompress) {
    std::string data = "Hello, World! Hello, World! Hello, World!";

    std::string compressed = atom::utils::compress(data);
    EXPECT_NE(compressed, data);

    std::string decompressed = atom::utils::decompress(compressed);
    EXPECT_EQ(decompressed, data);
}

// Tests for calculateSha256
TEST(HashTest, CalculateSha256) {
    std::string filename = "testfile.txt";

    // 创建一个测试文件
    std::ofstream outfile(filename);
    outfile << "Hello, World!";
    outfile.close();

    std::string hash = atom::utils::calculateSha256(filename);
    EXPECT_EQ(hash, "dummyhash");

    // 删除测试文件
    std::remove(filename.c_str());
}
