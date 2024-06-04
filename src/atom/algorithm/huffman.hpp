/*
 * huffman.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-24

Description: Simple implementation of Huffman encoding

**************************************************/

#ifndef ATOM_ALGORITHM_HUFFMAN_HPP
#define ATOM_ALGORITHM_HUFFMAN_HPP

#include <memory>
#include <string>
#include <unordered_map>

namespace atom::algorithm {
/**
 * @brief Represents a node in the Huffman tree.
 *
 * This structure is used to construct the Huffman tree for encoding and
 * decoding text based on character frequencies.
 */
struct HuffmanNode {
    char data;  ///< Character stored in this node (used only in leaf nodes).
    int frequency;  ///< Frequency of the character or sum of frequencies for
                    ///< internal nodes.
    std::shared_ptr<HuffmanNode> left;   ///< Pointer to the left child node.
    std::shared_ptr<HuffmanNode> right;  ///< Pointer to the right child node.

    /**
     * @brief Constructs a new Huffman Node.
     *
     * @param data Character to store in the node.
     * @param frequency Frequency of the character or combined frequency for a
     * parent node.
     */
    explicit HuffmanNode(char data, int frequency);
};

/**
 * @brief Creates a Huffman tree based on the frequency of characters.
 *
 * This function builds a Huffman tree using the frequencies of characters in
 * the input text. It employs a priority queue to build the tree from the bottom
 * up by merging the two least frequent nodes until only one node remains, which
 * becomes the root.
 *
 * @param frequencies A map of characters and their corresponding frequencies.
 * @return A shared pointer to the root of the Huffman tree.
 */
[[nodiscard]] std::shared_ptr<HuffmanNode> createHuffmanTree(
    const std::unordered_map<char, int>& frequencies);

/**
 * @brief Generates Huffman codes for each character from the Huffman tree.
 *
 * This function recursively traverses the Huffman tree and assigns a binary
 * code to each character. These codes are derived from the path taken to reach
 * the character: left child gives '0' and right child gives '1'.
 *
 * @param root Pointer to the root node of the Huffman tree.
 * @param code Current Huffman code generated during the traversal.
 * @param huffmanCodes A reference to a map where the character and its
 * corresponding Huffman code will be stored.
 */
void generateHuffmanCodes(const HuffmanNode* root, const std::string& code,
                          std::unordered_map<char, std::string>& huffmanCodes);

/**
 * @brief Compresses text using Huffman codes.
 *
 * This function converts a string of text into a string of binary codes based
 * on the Huffman codes provided. Each character in the input text is replaced
 * by its corresponding Huffman code.
 *
 * @param text The original text to compress.
 * @param huffmanCodes The map of characters to their corresponding Huffman
 * codes.
 * @return A string representing the compressed text.
 */
[[nodiscard]] std::string compressText(
    const std::string_view text,
    const std::unordered_map<char, std::string>& huffmanCodes);

/**
 * @brief Decompresses Huffman encoded text back to its original form.
 *
 * This function decodes a string of binary codes back into the original text
 * using the provided Huffman tree. It traverses the Huffman tree from the root
 * to the leaf nodes based on the binary string, reconstructing the original
 * text.
 *
 * @param compressedText The Huffman encoded text.
 * @param root Pointer to the root of the Huffman tree.
 * @return The original decompressed text.
 */
[[nodiscard]] std::string decompressText(const std::string_view compressedText,
                                         const HuffmanNode* root);

}  // namespace atom::algorithm

#endif
