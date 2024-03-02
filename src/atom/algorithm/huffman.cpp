/*
 * huffman.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-11-24

Description: Simple implementation of Huffman encoding

**************************************************/

#include "huffman.hpp"

#include <queue>

namespace Atom::Algorithm {
// 定义优先队列中的比较函数
struct Compare {
    bool operator()(HuffmanNode *a, HuffmanNode *b) {
        return a->frequency > b->frequency;
    }
};

// 创建哈夫曼树
HuffmanNode *createHuffmanTree(std::unordered_map<char, int> &frequencies) {
    std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, Compare>
        minHeap;

    // 将频率表中的字符和频率构建成叶子节点，并加入到最小堆
    for (auto &pair : frequencies) {
        char data = pair.first;
        int frequency = pair.second;
        HuffmanNode *node = new HuffmanNode(data, frequency);
        minHeap.push(node);
    }

    // 不断从最小堆中取出两个最小频率的节点，合并为一个新节点，并将新节点放回最小堆
    while (minHeap.size() > 1) {
        HuffmanNode *left = minHeap.top();
        minHeap.pop();
        HuffmanNode *right = minHeap.top();
        minHeap.pop();

        HuffmanNode *newNode =
            new HuffmanNode('$', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;

        minHeap.push(newNode);
    }

    // 返回最后剩下的根节点
    return minHeap.top();
}

// 递归生成哈夫曼编码
void generateHuffmanCodes(HuffmanNode *root, std::string code,
                          std::unordered_map<char, std::string> &huffmanCodes) {
    if (root->left == nullptr && root->right == nullptr) {
        huffmanCodes[root->data] = code;
        return;
    }

    generateHuffmanCodes(root->left, code + "0", huffmanCodes);
    generateHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// 使用哈夫曼编码压缩文本
std::string compressText(
    const std::string &text,
    const std::unordered_map<char, std::string> &huffmanCodes) {
    std::string compressedText = "";

    // 将原始文本中的每个字符替换为对应的哈夫曼编码
    for (char c : text) {
        compressedText += huffmanCodes.at(c);
    }

    return compressedText;
}

// 使用哈夫曼编码解压缩文本
std::string decompressText(const std::string &compressedText,
                           HuffmanNode *root) {
    std::string decompressedText = "";
    HuffmanNode *current = root;

    // 遍历压缩后的文本中的每个位，根据哈夫曼树进行解码
    for (char bit : compressedText) {
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (current->left == nullptr && current->right == nullptr) {
            decompressedText += current->data;
            current = root;
        }
    }

    return decompressedText;
}
}  // namespace Atom::Algorithm
