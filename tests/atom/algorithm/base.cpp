#include "atom/algorithm/base.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

// Test for Base16 encoding and decoding
TEST(Base16Test, EncodeDecode) {
    std::vector<unsigned char> data = {'f', 'o', 'o'};
    std::string encoded = atom::algorithm::base16Encode(data);
    EXPECT_EQ(encoded, "666F6F");

    std::vector<unsigned char> decoded = atom::algorithm::base16Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for Base32 encoding and decoding
TEST(Base32Test, EncodeDecode) {
    std::string data = "foo";
    std::string encoded = atom::algorithm::base32Encode(
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    EXPECT_EQ(encoded, "MZXW6===");  // Expected Base32 encoding of "foo"

    std::string decoded = atom::algorithm::base32Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for Base64 encoding and decoding
TEST(Base64Test, EncodeDecode) {
    std::string data = "foo";
    std::string encoded = atom::algorithm::base64Encode(data);
    EXPECT_EQ(encoded, "Zm9v");

    std::string decoded = atom::algorithm::base64Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for Base85 encoding and decoding
TEST(Base85Test, EncodeDecode) {
    std::vector<unsigned char> data = {'f', 'o', 'o'};
    std::string encoded = atom::algorithm::base85Encode(data);
    EXPECT_EQ(encoded, "FCfN8");  // Expected Base85 encoding of "foo"

    std::vector<unsigned char> decoded = atom::algorithm::base85Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for Base91 encoding and decoding
TEST(Base91Test, EncodeDecode) {
    std::string data = "foo";
    std::string encoded = atom::algorithm::base91Encode(data);
    EXPECT_EQ(encoded, "fLB~");  // Expected Base91 encoding of "foo"

    std::string decoded = atom::algorithm::base91Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for Base128 encoding and decoding
TEST(Base128Test, EncodeDecode) {
    std::string data = "foo";
    std::string encoded = atom::algorithm::base128Encode(
        reinterpret_cast<const uint8_t *>(data.data()), data.size());
    EXPECT_EQ(encoded,
              data);  // Base128 encoding is identical to input for ASCII

    std::string decoded = atom::algorithm::base128Decode(encoded);
    EXPECT_EQ(decoded, data);
}

// Test for XOR encryption and decryption
TEST(XOREncryptionTest, EncryptDecrypt) {
    std::string plaintext = "foo";
    uint8_t key = 0xAA;
    std::string ciphertext = atom::algorithm::xorEncrypt(plaintext, key);
    EXPECT_NE(ciphertext, plaintext);

    std::string decrypted = atom::algorithm::xorDecrypt(ciphertext, key);
    EXPECT_EQ(decrypted, plaintext);
}
