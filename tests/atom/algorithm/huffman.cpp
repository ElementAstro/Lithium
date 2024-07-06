#include "atom/algorithm/huffman.hpp"
#include <gtest/gtest.h>

// 创建Huffman树测试
TEST(HuffmanTest, CreateHuffmanTreeTest) {
    std::unordered_map<char, int> frequencies = {
        {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};

    auto root = atom::algorithm::createHuffmanTree(frequencies);
    ASSERT_EQ(root->frequency, 100);
    ASSERT_EQ(root->left->data, 'f');
}

TEST(HuffmanTest, GenerateHuffmanCodesTest) {
    auto root = std::make_shared<atom::algorithm::HuffmanNode>('\0', 0);
    root->left = std::make_shared<atom::algorithm::HuffmanNode>('\0', 0);
    root->right = std::make_shared<atom::algorithm::HuffmanNode>('\0', 0);

    std::unordered_map<char, std::string> huffmanCodes;
    atom::algorithm::generateHuffmanCodes(root.get(), "", huffmanCodes);

    ASSERT_EQ(huffmanCodes['\0'], "0");
}

// 压缩和解压缩文本测试
TEST(HuffmanTest, CompressAndDecompressTextTest) {
    std::string text = "abbcccddddeeeeeffffff";
    std::unordered_map<char, int> frequencies = {{'a', 1}, {'b', 2}, {'c', 3},
                                                 {'d', 4}, {'e', 5}, {'f', 6}};

    auto root = atom::algorithm::createHuffmanTree(frequencies);
    std::unordered_map<char, std::string> huffmanCodes;
    atom::algorithm::generateHuffmanCodes(root.get(), "", huffmanCodes);

    std::string compressedText =
        atom::algorithm::compressText(text, huffmanCodes);
    std::string decompressedText =
        atom::algorithm::decompressText(compressedText, root.get());

    ASSERT_EQ(decompressedText, text);
}
