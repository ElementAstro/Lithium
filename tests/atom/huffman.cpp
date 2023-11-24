#include <gtest/gtest.h>
#include <huffman.h>

class HuffmanTreeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test data
        frequencies = std::map<char, int>{
            {'a', 5},
            {'b', 3},
            {'c', 7},
            {'d', 2}};
        expectedHuffmanCodes = std::map<char, std::string>{
            {'a', "111"},
            {'b', "110"},
            {'c', "01"},
            {'d', "00"}};
        expectedCompressedText = "111110100100";
        expectedDecompressedText = "abcdd";
    }

    std::map<char, int> frequencies;
    std::map<char, std::string> expectedHuffmanCodes;
    std::string expectedCompressedText;
    std::string expectedDecompressedText;
};

TEST_F(HuffmanTreeTest, CreateHuffmanTree)
{
    HuffmanNode *root = createHuffmanTree(frequencies);

    // Verify root node
    EXPECT_EQ(root->data, '$');
    EXPECT_EQ(root->frequency, 17);

    // Verify left and right nodes
    HuffmanNode *leftNode = root->left;
    HuffmanNode *rightNode = root->right;
    EXPECT_NE(leftNode, nullptr);
    EXPECT_NE(rightNode, nullptr);
    EXPECT_EQ(leftNode->data, 'c');
    EXPECT_EQ(leftNode->frequency, 7);
    EXPECT_EQ(rightNode->data, 'a');
    EXPECT_EQ(rightNode->frequency, 5);
}

TEST_F(HuffmanTreeTest, GenerateHuffmanCodes)
{
    HuffmanNode *root = createHuffmanTree(frequencies);
    std::map<char, std::string> huffmanCodes;

    generateHuffmanCodes(root, "", huffmanCodes);

    // Verify generated huffman codes
    for (auto &pair : expectedHuffmanCodes)
    {
        EXPECT_EQ(huffmanCodes[pair.first], pair.second);
    }
}

TEST_F(HuffmanTreeTest, CompressText)
{
    HuffmanNode *root = createHuffmanTree(frequencies);
    std::string text = "abcdd";

    std::string compressedText = compressText(text, expectedHuffmanCodes);

    // Verify compressed text
    EXPECT_EQ(compressedText, expectedCompressedText);
}

TEST_F(HuffmanTreeTest, DecompressText)
{
    HuffmanNode *root = createHuffmanTree(frequencies);
    std::string compressedText = expectedCompressedText;

    std::string decompressedText = decompressText(compressedText, root);

    // Verify decompressed text
    EXPECT_EQ(decompressedText, expectedDecompressedText);
}