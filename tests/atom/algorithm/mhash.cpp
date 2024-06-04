#include "atom/algorithm/mhash.hpp"
#include <gtest/gtest.h>

// Tests for murmur3Hash
TEST(AlgorithmTest, Murmur3Hash) {
    EXPECT_EQ(atom::algorithm::murmur3Hash("test"),
              1060627423);  // Example hash value
    EXPECT_EQ(atom::algorithm::murmur3Hash("test", 12345),
              12345);  // Custom seed
}

// Tests for murmur3Hash64
TEST(AlgorithmTest, Murmur3Hash64) {
    EXPECT_EQ(atom::algorithm::murmur3Hash64("test"),
              (static_cast<uint64_t>(1060627423) << 32) |
                  1050126127);  // Example hash value
    EXPECT_EQ(atom::algorithm::murmur3Hash64("test", 12345, 67890),
              (static_cast<uint64_t>(12345) << 32) | 67890);  // Custom seeds
}

// Tests for hexstringFromData (void* version)
TEST(AlgorithmTest, HexstringFromDataVoid) {
    const char data[] = "test";
    char output[8];
    atom::algorithm::hexstringFromData(data, 4, output);
    EXPECT_EQ(std::string(output, 8), "74657374");  // "test" -> "74657374"
}

// Tests for hexstringFromData (string version)
TEST(AlgorithmTest, HexstringFromDataString) {
    std::string data = "test";
    std::string output = atom::algorithm::hexstringFromData(data);
    EXPECT_EQ(output, "74657374");  // "test" -> "74657374"
}

// Tests for dataFromHexstring
TEST(AlgorithmTest, DataFromHexstring) {
    std::string data = "74657374";
    std::string output = atom::algorithm::dataFromHexstring(data);
    EXPECT_EQ(output, "test");  // "74657374" -> "test"
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

TEST(AlgorithmTest, DataFromHexstringInvalid) {
    std::string data = "7465737";  // Invalid hexstring length
    EXPECT_THROW(atom::algorithm::dataFromHexstring(data),
                 std::invalid_argument);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
