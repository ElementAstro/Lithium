#include <gtest/gtest.h>
#include "atom/algorithm/huffman.hpp"

// 创建Huffman树测试
TEST(HuffmanTest, CreateHuffmanTreeTest)
{
    std::unordered_map<char, int> frequencies = {
        {'a', 5},
        {'b', 9},
        {'c', 12},
        {'d', 13},
        {'e', 16},
        {'f', 45}};

    Atom::Algorithm::HuffmanNode *root = Atom::Algorithm::createHuffmanTree(frequencies);

    // 验证树的结构和节点频率等信息是否符合预期
    ASSERT_EQ(root->frequency, 100);  // 总频率应该是所有字符频率之和
    ASSERT_EQ(root->left->data, 'f'); // 在此假设字符f具有最高的频率
    // 其他类似断言...
}

// 生成Huffman编码测试
TEST(HuffmanTest, GenerateHuffmanCodesTest)
{
    Atom::Algorithm::HuffmanNode *root = new Atom::Algorithm::HuffmanNode('\0', 0);
    root->left = new Atom::Algorithm::HuffmanNode('\0', 0);
    root->right = new Atom::Algorithm::HuffmanNode('\0', 0);

    std::unordered_map<char, std::string> huffmanCodes;
    Atom::Algorithm::generateHuffmanCodes(root, "", huffmanCodes);

    // 验证生成的Huffman编码是否符合预期
    ASSERT_EQ(huffmanCodes['\0'], "0"); // 示例断言
    // 其他类似断言...
}

// 压缩和解压缩文本测试
TEST(HuffmanTest, CompressAndDecompressTextTest)
{
    // 准备数据
    std::string text = "abbcccddddeeeeeffffff";
    std::unordered_map<char, int> frequencies = {
        {'a', 1},
        {'b', 2},
        {'c', 3},
        {'d', 4},
        {'e', 5},
        {'f', 6}};

    Atom::Algorithm::HuffmanNode *root = Atom::Algorithm::createHuffmanTree(frequencies);
    std::unordered_map<char, std::string> huffmanCodes;
    Atom::Algorithm::generateHuffmanCodes(root, "", huffmanCodes);

    // 执行压缩和解压缩
    std::string compressedText = Atom::Algorithm::compressText(text, huffmanCodes);
    std::string decompressedText = Atom::Algorithm::decompressText(compressedText, root);

    // 验证解压后的文本是否与原始文本一致
    ASSERT_EQ(decompressedText, text);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
