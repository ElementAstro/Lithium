#include "atom/utils/aes.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include "exception.hpp"
#include "macro.hpp"

using namespace atom::utils;

class AESTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any necessary resources
    }

    void TearDown() override {
        // Clean up any resources
    }
};

TEST_F(AESTest, EncryptionDecryption) {
    std::string plaintext = "This is a test plaintext.";
    std::string key = "1234567890123456";  // 16 bytes key for AES-128

    std::vector<unsigned char> iv;
    std::vector<unsigned char> tag;

    std::string ciphertext = encryptAES(plaintext, key, iv, tag);
    ASSERT_FALSE(ciphertext.empty());

    std::string decryptedtext = decryptAES(ciphertext, key, iv, tag);
    ASSERT_EQ(plaintext, decryptedtext);
}

TEST(CompressionTest, CompressDecompressSuccess) {
    std::string original =
        "Hello, World! This is a test of the zlib compression and "
        "decompression.";
    std::string compressed = compress(original);
    std::string decompressed = decompress(compressed);

    // Check that the decompressed data matches the original
    EXPECT_EQ(decompressed, original);
}

TEST(CompressionTest, CompressEmptyString) {
    // Compress an empty string
    EXPECT_THROW(ATOM_UNUSED_RESULT(compress("")),
                 atom::error::InvalidArgument);
}

TEST(CompressionTest, DecompressEmptyString) {
    EXPECT_THROW(ATOM_UNUSED_RESULT(decompress("")),
                 atom::error::InvalidArgument);
}

TEST(CompressionTest, CompressDifferentData) {
    std::string original1 = "Test compression 1.";
    std::string original2 = "Test compression 2.";

    std::string compressed1 = compress(original1);
    std::string compressed2 = compress(original2);

    // Ensure compressed outputs are different
    EXPECT_NE(compressed1, compressed2);
}

TEST(CompressionTest, DecompressInvalidData) {
    // Attempt to decompress invalid compressed data
    std::string invalidData = "This is not compressed data.";
    EXPECT_THROW(decompress(invalidData), atom::error::RuntimeError);
}

TEST(CompressionTest, CompressAndDecompressSpecialCharacters) {
    std::string original = "Special characters: !@#$%^&*()_+[]{}|;':\",.<>?";
    std::string compressed = compress(original);
    std::string decompressed = decompress(compressed);

    // Check that the decompressed data matches the original
    EXPECT_EQ(decompressed, original);
}

TEST(CompressionTest, CompressAndDecompressLongString) {
    std::string original(10000, 'A');  // Create a long string of 10,000 'A's
    std::string compressed = compress(original);
    std::string decompressed = decompress(compressed);

    // Check that the decompressed data matches the original
    EXPECT_EQ(decompressed, original);
}

TEST(CompressionTest, CompressDecompressBinaryData) {
    std::vector<uint8_t> originalData = {0x00, 0x01, 0x02, 0x03, 0x04,
                                         0x05, 0x06, 0x07, 0x08, 0x09};
    std::string_view original(
        reinterpret_cast<const char*>(originalData.data()),
        originalData.size());

    std::string compressed = compress(original);
    std::string decompressed = decompress(compressed);

    // Convert decompressed string back to vector
    std::vector<uint8_t> decompressedData(decompressed.begin(),
                                          decompressed.end());

    // Check that the decompressed data matches the original
    EXPECT_EQ(originalData.size(), decompressedData.size());
    EXPECT_TRUE(std::equal(originalData.begin(), originalData.end(),
                           decompressedData.begin()));
}

TEST(CompressionTest, CompressEmptyBinaryData) {
    EXPECT_THROW(ATOM_UNUSED_RESULT(compress("")),
                 atom::error::InvalidArgument);
}

TEST(CompressionTest, DecompressInvalidBinaryData) {
    std::string invalidData = "This is not compressed data.";
    EXPECT_THROW(ATOM_UNUSED_RESULT(decompress(invalidData)),
                 atom::error::RuntimeError);
}

TEST(CompressionTest, CompressSpecialBinaryData) {
    std::vector<uint8_t> originalData = {0xFF, 0xFE, 0xFD, 0xFC};
    std::string_view original(
        reinterpret_cast<const char*>(originalData.data()),
        originalData.size());

    std::string compressed = compress(original);
    std::string decompressed = decompress(compressed);

    // Convert decompressed string back to vector
    std::vector<uint8_t> decompressedData(decompressed.begin(),
                                          decompressed.end());

    // Check that the decompressed data matches the original
    EXPECT_EQ(originalData.size(), decompressedData.size());
    EXPECT_TRUE(std::equal(originalData.begin(), originalData.end(),
                           decompressedData.begin()));
}