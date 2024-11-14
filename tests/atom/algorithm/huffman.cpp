#include "atom/algorithm/huffman.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

class HuffmanTest : public ::testing::Test {
protected:
    // Helper function to create a sample frequency map
    static auto createSampleFrequencies() {
        return std::unordered_map<unsigned char, int>{
            {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};
    }

    // Helper function to create a simple test tree
    static auto createTestTree() {
        auto root = std::make_shared<HuffmanNode>('\0', 10);
        root->left = std::make_shared<HuffmanNode>('a', 4);
        root->right = std::make_shared<HuffmanNode>('b', 6);
        return root;
    }
};

// HuffmanNode Tests
TEST_F(HuffmanTest, NodeConstruction) {
    HuffmanNode node('a', 5);
    EXPECT_EQ(node.data, 'a');
    EXPECT_EQ(node.frequency, 5);
    EXPECT_EQ(node.left, nullptr);
    EXPECT_EQ(node.right, nullptr);
}

// Tree Creation Tests
TEST_F(HuffmanTest, CreateHuffmanTreeEmpty) {
    std::unordered_map<unsigned char, int> emptyFreq;
    EXPECT_THROW(createHuffmanTree(emptyFreq), HuffmanException);
}

TEST_F(HuffmanTest, CreateHuffmanTreeSingleChar) {
    std::unordered_map<unsigned char, int> freq{{'a', 1}};
    auto root = createHuffmanTree(freq);
    EXPECT_EQ(root->data, 'a');
    EXPECT_EQ(root->frequency, 1);
}

TEST_F(HuffmanTest, CreateHuffmanTreeMultipleChars) {
    auto freq = createSampleFrequencies();
    auto root = createHuffmanTree(freq);
    EXPECT_NE(root, nullptr);
    EXPECT_GT(root->frequency, 0);
}

// Code Generation Tests
TEST_F(HuffmanTest, GenerateHuffmanCodesEmpty) {
    std::unordered_map<unsigned char, std::string> codes;
    generateHuffmanCodes(nullptr, "", codes);
    EXPECT_TRUE(codes.empty());
}

TEST_F(HuffmanTest, GenerateHuffmanCodesSimple) {
    auto root = createTestTree();
    std::unordered_map<unsigned char, std::string> codes;
    generateHuffmanCodes(root.get(), "", codes);
    EXPECT_EQ(codes['a'], "0");
    EXPECT_EQ(codes['b'], "1");
}

// Compression Tests
TEST_F(HuffmanTest, CompressDataEmpty) {
    std::vector<unsigned char> emptyData;
    std::unordered_map<unsigned char, std::string> codes;
    auto compressed = compressData(emptyData, codes);
    EXPECT_TRUE(compressed.empty());
}

TEST_F(HuffmanTest, CompressDataSimple) {
    std::vector<unsigned char> data{'a', 'b', 'a'};
    std::unordered_map<unsigned char, std::string> codes{{'a', "0"},
                                                         {'b', "1"}};
    auto compressed = compressData(data, codes);
    EXPECT_EQ(compressed, "010");
}

TEST_F(HuffmanTest, CompressDataInvalidCode) {
    std::vector<unsigned char> data{'x'};
    std::unordered_map<unsigned char, std::string> codes{{'a', "0"},
                                                         {'b', "1"}};
    EXPECT_THROW(compressData(data, codes), HuffmanException);
}

// Decompression Tests
TEST_F(HuffmanTest, DecompressDataEmpty) {
    auto root = createTestTree();
    auto decompressed = decompressData("", root.get());
    EXPECT_TRUE(decompressed.empty());
}

TEST_F(HuffmanTest, DecompressDataSimple) {
    auto root = createTestTree();
    auto decompressed = decompressData("01", root.get());
    EXPECT_EQ(decompressed.size(), 2);
    EXPECT_EQ(decompressed[0], 'a');
    EXPECT_EQ(decompressed[1], 'b');
}

TEST_F(HuffmanTest, DecompressDataInvalid) {
    EXPECT_THROW(decompressData("01", nullptr), HuffmanException);
}

// Serialization Tests
TEST_F(HuffmanTest, SerializeTreeEmpty) {
    EXPECT_TRUE(serializeTree(nullptr).empty());
}

TEST_F(HuffmanTest, SerializeTreeSimple) {
    auto root = createTestTree();
    auto serialized = serializeTree(root.get());
    EXPECT_FALSE(serialized.empty());
}

// Deserialization Tests
TEST_F(HuffmanTest, DeserializeTreeEmpty) {
    size_t index = 0;
    EXPECT_THROW(deserializeTree("", index), HuffmanException);
}

TEST_F(HuffmanTest, DeserializeTreeSimple) {
    auto root = createTestTree();
    auto serialized = serializeTree(root.get());
    size_t index = 0;
    auto deserialized = deserializeTree(serialized, index);
    EXPECT_NE(deserialized, nullptr);
}

// Integration Tests
TEST_F(HuffmanTest, FullEncodingDecodingCycle) {
    // Original data
    std::vector<unsigned char> original{'h', 'e', 'l', 'l', 'o'};

    // Create frequency map
    std::unordered_map<unsigned char, int> frequencies;
    for (auto c : original) {
        frequencies[c]++;
    }

    // Create tree and generate codes
    auto root = createHuffmanTree(frequencies);
    std::unordered_map<unsigned char, std::string> codes;
    generateHuffmanCodes(root.get(), "", codes);

    // Compress
    auto compressed = compressData(original, codes);

    // Decompress
    auto decompressed = decompressData(compressed, root.get());

    // Verify
    EXPECT_EQ(original, decompressed);
}

// Visualization Test
TEST_F(HuffmanTest, VisualizeTree) {
    auto root = createTestTree();
    testing::internal::CaptureStdout();
    visualizeHuffmanTree(root.get());
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(output.empty());
}

// Error Handling Tests
TEST_F(HuffmanTest, ExceptionMessages) {
    try {
        std::unordered_map<unsigned char, int> emptyFreq;
        createHuffmanTree(emptyFreq);
        FAIL() << "Expected HuffmanException";
    } catch (const HuffmanException& e) {
        EXPECT_NE(std::string(e.what()), "");
    }
}