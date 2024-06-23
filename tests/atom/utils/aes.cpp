#include "atom/utils/aes.hpp"
#include <gtest/gtest.h>
#include <fstream>

namespace {

class AESTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any necessary resources
    }

    void TearDown() override {
        // Clean up any resources
    }
};

TEST_F(AESTest, EncryptDecryptAES) {
    std::string plaintext = "Hello, World!";
    std::string key = "0123456789abcdef";

    std::string ciphertext = atom::utils::encryptAES(plaintext, key);
    EXPECT_NE(ciphertext, plaintext);

    std::string decrypted = atom::utils::decryptAES(ciphertext, key);
    EXPECT_EQ(decrypted, plaintext);
}

TEST_F(AESTest, EncryptDecryptAESEmptyString) {
    std::string plaintext = "";
    std::string key = "0123456789abcdef";

    std::string ciphertext = atom::utils::encryptAES(plaintext, key);
    EXPECT_NE(ciphertext, plaintext);

    std::string decrypted = atom::utils::decryptAES(ciphertext, key);
    EXPECT_EQ(decrypted, plaintext);
}

TEST_F(AESTest, CompressDecompress) {
    std::string original =
        "This is a test string that will be compressed and then decompressed.";

    std::string compressed = atom::utils::compress(original);
    EXPECT_NE(compressed, original);
    EXPECT_LT(compressed.length(), original.length());

    std::string decompressed = atom::utils::decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST_F(AESTest, CompressDecompressEmptyString) {
    std::string original = "";

    std::string compressed = atom::utils::compress(original);
    EXPECT_EQ(compressed, original);

    std::string decompressed = atom::utils::decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST_F(AESTest, CalculateSha256) {
    // Create a temporary file
    std::string filename = "test_file.txt";
    std::string content = "This is a test file for SHA-256 calculation.";

    std::ofstream file(filename);
    file << content;
    file.close();

    std::string hash = atom::utils::calculateSha256(filename);
    EXPECT_EQ(hash.length(), 64);  // SHA-256 hash is 64 characters long

    // Verify that the hash is consistent
    std::string hash2 = atom::utils::calculateSha256(filename);
    EXPECT_EQ(hash, hash2);

    // Remove the temporary file
    std::remove(filename.c_str());
}

TEST_F(AESTest, CalculateSha256NonExistentFile) {
    std::string filename = "non_existent_file.txt";
    EXPECT_THROW(atom::utils::calculateSha256(filename), std::runtime_error);
}

}  // namespace