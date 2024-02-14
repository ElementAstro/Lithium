/*
 * huffman.hpp
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

#ifndef ATOM_ALGORITHM_HUFFMAN_HPP
#define ATOM_ALGORITHM_HUFFMAN_HPP

#include <string>
#include <unordered_map>

namespace Atom::Algorithm
{
    /**
     * @brief 哈夫曼树节点
     *
     */
    struct HuffmanNode
    {
        char data;          ///< 节点存储的字符
        int frequency;      ///< 节点的频率
        HuffmanNode *left;  ///< 左子节点指针
        HuffmanNode *right; ///< 右子节点指针

        /**
         * @brief 构造函数
         *
         * @param data 节点存储的字符
         * @param frequency 节点的频率
         */
        HuffmanNode(char data, int frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
    };

    /**
     * @brief 创建哈夫曼树
     *
     * @param frequencies 字符频率表
     * @return HuffmanNode* 哈夫曼树的根节点指针
     */
    HuffmanNode *createHuffmanTree(std::unordered_map<char, int> &frequencies);

    /**
     * @brief 递归生成哈夫曼编码
     *
     * @param root 当前节点指针
     * @param code 当前节点编码
     * @param huffmanCodes 哈夫曼编码表
     */
    void generateHuffmanCodes(HuffmanNode *root, std::string code, std::unordered_map<char, std::string> &huffmanCodes);

    /**
     * @brief 使用哈夫曼编码压缩文本
     *
     * @param text 待压缩文本
     * @param huffmanCodes 哈夫曼编码表
     * @return std::string 压缩后的文本
     */
    std::string compressText(const std::string &text, const std::unordered_map<char, std::string> &huffmanCodes);

    /**
     * @brief 使用哈夫曼编码解压缩文本
     *
     * @param compressedText 压缩后的文本
     * @param root 哈夫曼树的根节点指针
     * @return std::string 解压缩后的文本
     */
    std::string decompressText(const std::string &compressedText, HuffmanNode *root);
}

#endif