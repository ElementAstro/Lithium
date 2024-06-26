#include <gtest/gtest.h>

#include "atom/utils/bit.hpp"

TEST(BitManipulation, CreateMask) {
    EXPECT_EQ(atom::utils::CreateMask<uint8_t>(3), 0x07);
    EXPECT_EQ(atom::utils::CreateMask<uint16_t>(10), 0x03FF);
    EXPECT_EQ(atom::utils::CreateMask<uint32_t>(32), 0xFFFFFFFF);
}

TEST(BitManipulation, CountBytes) {
    EXPECT_EQ(atom::utils::CountBytes<uint8_t>(0x55), 4);
    EXPECT_EQ(atom::utils::CountBytes<uint16_t>(0x5555), 8);
    EXPECT_EQ(atom::utils::CountBytes<uint32_t>(0x55555555), 16);
}

TEST(BitManipulation, ReverseBits) {
    EXPECT_EQ(atom::utils::ReverseBits<uint8_t>(0b00000011), 0b11000000);
    EXPECT_EQ(atom::utils::ReverseBits<uint16_t>(0x8001),
              0x8001);  // Palindrome pattern
}

TEST(BitManipulation, RotateBits) {
    EXPECT_EQ(atom::utils::RotateLeft<uint8_t>(0x81, 1), 0x03);
    EXPECT_EQ(atom::utils::RotateRight<uint8_t>(0x81, 1), 0xC0);
}