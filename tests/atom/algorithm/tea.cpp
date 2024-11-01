#include "atom/algorithm/tea.hpp"

#include <gtest/gtest.h>
#include <array>
#include <vector>

using namespace atom::algorithm;

// Test data
constexpr std::array<uint32_t, 4> kKey = {0x12345678, 0x9abcdef0, 0xfedcba98,
                                          0x76543210};

auto getPlaintext() -> std::vector<uint8_t> {
    return {'T', 'e', 's', 't', 'T', 'E', 'A'};
}

auto getUint32Vector() -> std::vector<uint32_t> {
    return toUint32Vector(getPlaintext());
}

// Test TEA encryption and decryption
TEST(TEATest, EncryptDecrypt) {
    auto uint32Vector = getUint32Vector();
    uint32_t value0 = uint32Vector[0];
    uint32_t value1 = uint32Vector[1];

    teaEncrypt(value0, value1, kKey);
    teaDecrypt(value0, value1, kKey);

    EXPECT_EQ(value0, uint32Vector[0]);
    EXPECT_EQ(value1, uint32Vector[1]);
}

// Test XXTEA encryption and decryption
TEST(XXTEATest, EncryptDecrypt) {
    auto uint32Vector = getUint32Vector();
    std::vector<uint32_t> encrypted = xxteaEncrypt(
        uint32Vector, {0x12345678, 0x9abcdef0, 0xfedcba98, 0x76543210});
    std::vector<uint32_t> decrypted = xxteaDecrypt(
        encrypted, {0x12345678, 0x9abcdef0, 0xfedcba98, 0x76543210});

    EXPECT_EQ(decrypted, uint32Vector);
}

// Test XTEA encryption and decryption
TEST(XTEATest, EncryptDecrypt) {
    auto uint32Vector = getUint32Vector();
    uint32_t value0 = uint32Vector[0];
    uint32_t value1 = uint32Vector[1];

    xteaEncrypt(value0, value1, kKey);
    xteaDecrypt(value0, value1, kKey);

    EXPECT_EQ(value0, uint32Vector[0]);
    EXPECT_EQ(value1, uint32Vector[1]);
}

// Test conversion from byte array to uint32_t vector
TEST(ConversionTest, ToUint32Vector) {
    auto plaintext = getPlaintext();
    std::vector<uint32_t> result = toUint32Vector(plaintext);
    EXPECT_EQ(result, getUint32Vector());
}

// Test conversion from uint32_t vector to byte array
TEST(ConversionTest, ToByteArray) {
    auto uint32Vector = getUint32Vector();
    std::vector<uint8_t> result = toByteArray(uint32Vector);
    EXPECT_EQ(result, getPlaintext());
}
