#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "atom/algorithm/base.hpp"


using namespace Atom::Algorithm;

class Base16Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                    0x57, 0x6f, 0x72, 0x6c, 0x64};
        testEncodedData = "48656c6c6f20576f726c64";
    }

    std::vector<unsigned char> testData;
    std::string testEncodedData;
};

TEST_F(Base16Test, EncodeBase16) {
    // Encode test data
    std::string encodedData = encodeBase16(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testEncodedData);
}

TEST_F(Base16Test, DecodeBase16) {
    // Decode test data
    std::vector<unsigned char> decodedData = decodeBase16(testEncodedData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}

class Base32Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = "Hello World!";
        testEncodedData = "JBSWY3DPEHPK3PXP";
    }

    std::string testData;
    std::string testEncodedData;
};

TEST_F(Base32Test, EncodeBase32) {
    // Encode test data
    std::string encodedData = encodeBase32(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testEncodedData);
}

TEST_F(Base32Test, DecodeBase32) {
    // Decode test data
    std::string decodedData = decodeBase32(testEncodedData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}

class Base64Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                    0x57, 0x6f, 0x72, 0x6c, 0x64};
        testEncodedData = "SGVsbG8gV29ybGQ=";
    }

    std::vector<unsigned char> testData;
    std::string testEncodedData;
};

TEST_F(Base64Test, Base64Encode) {
    // Encode test data
    std::string encodedData = base64Encode(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testEncodedData);
}

TEST_F(Base64Test, Base64Decode) {
    // Decode test data
    std::vector<unsigned char> decodedData = base64Decode(testEncodedData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}

class Base64EnhanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                    0x57, 0x6f, 0x72, 0x6c, 0x64};
        testEncodedData = "SGVsbG8gV29ybGQ=";
    }

    std::vector<uint8_t> testData;
    std::string testEncodedData;
};

TEST_F(Base64EnhanceTest, Base64EncodeEnhance) {
    // Encode test data
    std::string encodedData = base64EncodeEnhance(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testEncodedData);
}

#ifndef __MAIN__
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif


TEST_F(Base64EnhanceTest, Base64DecodeEnhance) {
    // Decode test data
    std::vector<uint8_t> decodedData = base64DecodeEnhance(testEncodedData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}

class Base85Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                    0x57, 0x6f, 0x72, 0x6c, 0x64};
        testEncodedData = "Hello World!";
    }

    std::vector<unsigned char> testData;
    std::string testEncodedData;
};

TEST_F(Base85Test, EncodeBase85) {
    // Encode test data
    std::string encodedData = encodeBase85(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testEncodedData);
}

TEST_F(Base85Test, DecodeBase85) {
    // Decode test data
    std::vector<unsigned char> decodedData = decodeBase85(testEncodedData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}

class Base128Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testData = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20,
                    0x57, 0x6f, 0x72, 0x6c, 0x64};
    }

    std::vector<uint8_t> testData;
};

TEST_F(Base128Test, EncodeBase128) {
    // Encode test data
    std::vector<uint8_t> encodedData = encodeBase128(testData);

    // Check the encoded data
    EXPECT_EQ(encodedData, testData);
}

TEST_F(Base128Test, DecodeBase128) {
    // Decode test data
    std::vector<uint8_t> decodedData = decodeBase128(testData);

    // Check the decoded data
    EXPECT_EQ(decodedData, testData);
}
