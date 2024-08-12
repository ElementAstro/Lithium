#include "atom/algorithm/mhash.hpp"
#include <gtest/gtest.h>

#include <set>
#include "exception.hpp"

using namespace atom::algorithm;
// Tests for hexstringFromData (string version)
TEST(AlgorithmTest, HexstringFromDataString) {
    std::string data = "test";
    std::string output = hexstringFromData(data);
    EXPECT_EQ(output, "74657374");  // "test" -> "74657374"
}

// Tests for dataFromHexstring
TEST(AlgorithmTest, DataFromHexstring) {
    std::string data = "74657374";
    std::string output = dataFromHexstring(data);
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
    EXPECT_THROW(dataFromHexstring(data), atom::error::InvalidArgument);
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

class MinHashTest : public ::testing::Test {
protected:
    MinHash minhash{100};

    std::set<int> set1{1, 2, 3, 4, 5};
    std::set<int> set2{4, 5, 6, 7, 8};
    std::set<int> set3{1, 2, 3, 4, 5};  // Same as set1

    void SetUp() override {
        // Custom setup code can go here
    }
};

TEST_F(MinHashTest, ComputeSignature) {
    auto signature1 = minhash.computeSignature(set1);
    auto signature2 = minhash.computeSignature(set2);

    EXPECT_EQ(signature1.size(), 100);
    EXPECT_EQ(signature2.size(), 100);

    // Since signatures should have minimum values, they should be less than max
    // size_t
    for (const auto& val : signature1) {
        EXPECT_LT(val, std::numeric_limits<size_t>::max());
    }
    for (const auto& val : signature2) {
        EXPECT_LT(val, std::numeric_limits<size_t>::max());
    }
}

TEST_F(MinHashTest, ComputeSignatureSameSets) {
    auto signature1 = minhash.computeSignature(set1);
    auto signature3 = minhash.computeSignature(set3);

    EXPECT_EQ(signature1, signature3);
}

TEST_F(MinHashTest, JaccardIndex) {
    auto signature1 = minhash.computeSignature(set1);
    auto signature2 = minhash.computeSignature(set2);

    double similarity = MinHash::jaccardIndex(signature1, signature2);
    EXPECT_GE(similarity, 0.0);
    EXPECT_LE(similarity, 1.0);
}

TEST_F(MinHashTest, JaccardIndexSameSets) {
    auto signature1 = minhash.computeSignature(set1);
    auto signature3 = minhash.computeSignature(set3);

    double similarity = MinHash::jaccardIndex(signature1, signature3);
    EXPECT_DOUBLE_EQ(similarity, 1.0);
}
