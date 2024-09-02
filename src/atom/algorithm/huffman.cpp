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

#ifdef USE_OPENMP
#include <omp.h>
#endif

namespace atom::algorithm {
HuffmanNode::HuffmanNode(char data, int frequency)
    : data(data), frequency(frequency), left(nullptr), right(nullptr) {}

auto createHuffmanTree(const std::unordered_map<char, int>& frequencies)
    -> std::shared_ptr<HuffmanNode> {
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

    return minHeap.empty() ? nullptr : minHeap.top();
}

void generateHuffmanCodes(const HuffmanNode* root, const std::string& code,
                          std::unordered_map<char, std::string>& huffmanCodes) {
    if (root == nullptr) {
        return;
    }
    if (!root->left && !root->right) {
        huffmanCodes[root->data] = code;
    } else {
        generateHuffmanCodes(root->left.get(), code + "0", huffmanCodes);
        generateHuffmanCodes(root->right.get(), code + "1", huffmanCodes);
    }
    if (!root->left && !root->right) {
        huffmanCodes[root->data] = code;
    } else {
        generateHuffmanCodes(root->left.get(), code + "0", huffmanCodes);
        generateHuffmanCodes(root->right.get(), code + "1", huffmanCodes);
    }
}

auto compressText(std::string_view TEXT,
                  const std::unordered_map<char, std::string>& huffmanCodes)
    -> std::string {
    std::string compressedText;

#ifdef USE_OPENMP
#pragma omp parallel
    {
        std::string local_compressed;
#pragma omp for nowait schedule(static)
        for (std::size_t i = 0; i < TEXT.size(); ++i) {
            local_compressed += huffmanCodes.at(TEXT[i]);
        }
#pragma omp critical
        compressedText += local_compressed;
    }
#else
    for (char c : TEXT) {
        compressedText += huffmanCodes.at(c);
    }
#endif

    return compressedText;
}

auto decompressText(std::string_view COMPRESSED_TEXT,
                    const HuffmanNode* root) -> std::string {
    std::string decompressedText;
    const HuffmanNode* current = root;

    for (char bit : COMPRESSED_TEXT) {
        current = (bit == '0') ? current->left.get() : current->right.get();
        if ((current != nullptr) && !current->left && !current->right) {
            decompressedText += current->data;
            current = root;
        }
    }

    return decompressedText;
}
}  // namespace atom::algorithm
