/*
 * huffman.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-24

Description: Simple implementation of Huffman encoding

**************************************************/

#include "huffman.hpp"

#include <queue>

namespace atom::algorithm {
HuffmanNode::HuffmanNode(char data, int frequency)
    : data(data), frequency(frequency), left(nullptr), right(nullptr) {}

std::shared_ptr<HuffmanNode> createHuffmanTree(
    const std::unordered_map<char, int>& frequencies) {
    auto compare = [](const std::shared_ptr<HuffmanNode>& a,
                      const std::shared_ptr<HuffmanNode>& b) {
        return a->frequency > b->frequency;
    };

    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>,
                        decltype(compare)>
        minHeap(compare);

    for (const auto& [data, frequency] : frequencies) {
        minHeap.push(std::make_unique<HuffmanNode>(data, frequency));
    }

    while (minHeap.size() > 1) {
        auto left = minHeap.top();
        minHeap.pop();
        auto right = minHeap.top();
        minHeap.pop();

        auto newNode = std::make_unique<HuffmanNode>(
            '$', left->frequency + right->frequency);
        newNode->left = std::move(left);
        newNode->right = std::move(right);

        minHeap.push(std::move(newNode));
    }

    return minHeap.empty() ? nullptr : std::move(minHeap.top());
}

void generateHuffmanCodes(const HuffmanNode* root, const std::string& code,
                          std::unordered_map<char, std::string>& huffmanCodes) {
    if (!root)
        return;
    if (!root->left && !root->right) {
        huffmanCodes[root->data] = code;
    } else {
        generateHuffmanCodes(root->left.get(), code + "0", huffmanCodes);
        generateHuffmanCodes(root->right.get(), code + "1", huffmanCodes);
    }
}

std::string compressText(
    const std::string_view text,
    const std::unordered_map<char, std::string>& huffmanCodes) {
    std::string compressedText;
    for (char c : text) {
        compressedText += huffmanCodes.at(c);
    }
    return compressedText;
}

std::string decompressText(const std::string_view compressedText,
                           const HuffmanNode* root) {
    std::string decompressedText;
    const HuffmanNode* current = root;

    for (char bit : compressedText) {
        current = (bit == '0') ? current->left.get() : current->right.get();
        if (current && !current->left && !current->right) {
            decompressedText += current->data;
            current = root;
        }
    }

    return decompressedText;
}
}  // namespace atom::algorithm
