/*
 * huffman.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-24

Description: Enhanced implementation of Huffman encoding

**************************************************/

#include "huffman.hpp"
#include <bitset>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>

namespace atom::algorithm {

/* ------------------------ HuffmanNode Implementation ------------------------
 */

HuffmanNode::HuffmanNode(unsigned char data, int frequency)
    : data(data), frequency(frequency), left(nullptr), right(nullptr) {}

/* ------------------------ Priority Queue Comparator ------------------------
 */

struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
                    const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency;
    }
};

/* ------------------------ createHuffmanTree ------------------------ */

auto createHuffmanTree(const std::unordered_map<unsigned char, int>&
                           frequencies) -> std::shared_ptr<HuffmanNode> {
    if (frequencies.empty()) {
        throw HuffmanException(
            "Frequency map is empty. Cannot create Huffman Tree.");
    }

    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>, CompareNode>
        minHeap;

    // Initialize heap with leaf nodes
    for (const auto& [data, freq] : frequencies) {
        minHeap.push(std::make_shared<HuffmanNode>(data, freq));
    }

    // Edge case: Only one unique byte
    if (minHeap.size() == 1) {
        auto soleNode = std::move(minHeap.top());
        minHeap.pop();
        auto parent = std::make_shared<HuffmanNode>('\0', soleNode->frequency);
        parent->left = std::move(soleNode);
        parent->right = nullptr;
        minHeap.push(std::move(parent));
    }

    // Build Huffman Tree
    while (minHeap.size() > 1) {
        auto left = std::move(minHeap.top());
        minHeap.pop();
        auto right = std::move(minHeap.top());
        minHeap.pop();

        auto merged = std::make_shared<HuffmanNode>(
            '\0', left->frequency + right->frequency);
        merged->left = std::move(left);
        merged->right = std::move(right);

        minHeap.push(std::move(merged));
    }

    return minHeap.empty() ? nullptr : std::move(minHeap.top());
}

/* ------------------------ generateHuffmanCodes ------------------------ */

void generateHuffmanCodes(
    const HuffmanNode* root, const std::string& code,
    std::unordered_map<unsigned char, std::string>& huffmanCodes) {
    if (root == nullptr) {
        throw HuffmanException(
            "Cannot generate Huffman codes from a null tree.");
    }

    if (!root->left && !root->right) {
        if (code.empty()) {
            // Edge case: Only one unique byte
            huffmanCodes[root->data] = "0";
        } else {
            huffmanCodes[root->data] = code;
        }
        return;
    }

    if (root->left) {
        generateHuffmanCodes(root->left.get(), code + "0", huffmanCodes);
    }

    if (root->right) {
        generateHuffmanCodes(root->right.get(), code + "1", huffmanCodes);
    }
}

/* ------------------------ compressData ------------------------ */

auto compressData(const std::vector<unsigned char>& data,
                  const std::unordered_map<unsigned char, std::string>&
                      huffmanCodes) -> std::string {
    std::string compressedData;
    compressedData.reserve(data.size() * 2);  // Approximate reserve

    for (unsigned char byte : data) {
        auto it = huffmanCodes.find(byte);
        if (it == huffmanCodes.end()) {
            throw HuffmanException(
                std::string("Byte '") + std::to_string(static_cast<int>(byte)) +
                "' does not have a corresponding Huffman code.");
        }
        compressedData += it->second;
    }

    return compressedData;
}

/* ------------------------ decompressData ------------------------ */

auto decompressData(const std::string& compressedData,
                    const HuffmanNode* root) -> std::vector<unsigned char> {
    if (!root) {
        throw HuffmanException("Huffman tree is null. Cannot decompress data.");
    }

    std::vector<unsigned char> decompressedData;
    const HuffmanNode* current = root;

    for (char bit : compressedData) {
        if (bit == '0') {
            if (current->left) {
                current = current->left.get();
            } else {
                throw HuffmanException(
                    "Invalid compressed data. Traversed to a null left child.");
            }
        } else if (bit == '1') {
            if (current->right) {
                current = current->right.get();
            } else {
                throw HuffmanException(
                    "Invalid compressed data. Traversed to a null right "
                    "child.");
            }
        } else {
            throw HuffmanException(
                "Invalid bit in compressed data. Only '0' and '1' are "
                "allowed.");
        }

        // If leaf node, append the data and reset to root
        if (!current->left && !current->right) {
            decompressedData.push_back(current->data);
            current = root;
        }
    }

    // Edge case: compressed data does not end at a leaf node
    if (current != root) {
        throw HuffmanException(
            "Incomplete compressed data. Did not end at a leaf node.");
    }

    return decompressedData;
}

/* ------------------------ serializeTree ------------------------ */

auto serializeTree(const HuffmanNode* root) -> std::string {
    if (root == nullptr) {
        throw HuffmanException("Cannot serialize a null Huffman tree.");
    }

    std::string serialized;
    std::function<void(const HuffmanNode*)> serializeHelper =
        [&](const HuffmanNode* node) {
            if (!node) {
                serialized += '1';  // Marker for null
                return;
            }

            if (!node->left && !node->right) {
                serialized += '0';  // Marker for leaf
                serialized += node->data;
            } else {
                serialized += '2';  // Marker for internal node
                serializeHelper(node->left.get());
                serializeHelper(node->right.get());
            }
        };

    serializeHelper(root);
    return serialized;
}

/* ------------------------ deserializeTree ------------------------ */

auto deserializeTree(const std::string& serializedTree,
                     size_t& index) -> std::shared_ptr<HuffmanNode> {
    if (index >= serializedTree.size()) {
        throw HuffmanException(
            "Invalid serialized tree format: Unexpected end of data.");
    }

    char marker = serializedTree[index++];
    if (marker == '1') {
        return nullptr;
    } else if (marker == '0') {
        if (index >= serializedTree.size()) {
            throw HuffmanException(
                "Invalid serialized tree format: Missing byte data for leaf "
                "node.");
        }
        unsigned char data = serializedTree[index++];
        return std::make_shared<HuffmanNode>(
            data, 0);  // Frequency is not needed for decompression
    } else if (marker == '2') {
        auto node = std::make_shared<HuffmanNode>('\0', 0);
        node->left = deserializeTree(serializedTree, index);
        node->right = deserializeTree(serializedTree, index);
        return node;
    } else {
        throw HuffmanException(
            "Invalid serialized tree format: Unknown marker encountered.");
    }
}

/* ------------------------ visualizeHuffmanTree ------------------------ */

void visualizeHuffmanTree(const HuffmanNode* root, const std::string& indent) {
    if (!root) {
        std::cout << indent << "nullptr\n";
        return;
    }

    if (!root->left && !root->right) {
        std::cout << indent << "Leaf: '" << root->data << "'\n";
    } else {
        std::cout << indent << "Internal Node (Frequency: " << root->frequency
                  << ")\n";
    }

    if (root->left) {
        std::cout << indent << " Left:\n";
        visualizeHuffmanTree(root->left.get(), indent + "  ");
    } else {
        std::cout << indent << " Left: nullptr\n";
    }

    if (root->right) {
        std::cout << indent << " Right:\n";
        visualizeHuffmanTree(root->right.get(), indent + "  ");
    } else {
        std::cout << indent << " Right: nullptr\n";
    }
}

}  // namespace atom::algorithm