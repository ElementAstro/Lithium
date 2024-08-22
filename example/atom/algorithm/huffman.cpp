#include "atom/algorithm/huffman.hpp"

#include <iostream>
#include <unordered_map>

int main() {
    {
        // Frequency map for characters
        std::unordered_map<char, int> frequencies = {
            {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};

        // Create Huffman Tree
        auto huffmanTree = atom::algorithm::createHuffmanTree(frequencies);

        if (huffmanTree) {
            std::cout << "Huffman tree created successfully." << std::endl;
        }
    }

    {
        // Example frequency map
        std::unordered_map<char, int> frequencies = {
            {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};

        // Create Huffman Tree
        auto huffmanTree = atom::algorithm::createHuffmanTree(frequencies);

        // Generate Huffman Codes
        std::unordered_map<char, std::string> huffmanCodes;
        atom::algorithm::generateHuffmanCodes(huffmanTree.get(), "",
                                              huffmanCodes);

        // Print Huffman Codes
        for (const auto& pair : huffmanCodes) {
            std::cout << "Character: " << pair.first
                      << ", Code: " << pair.second << std::endl;
        }
    }

    {
        // Example frequency map
        std::unordered_map<char, int> frequencies = {
            {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};

        // Create Huffman Tree
        auto huffmanTree = atom::algorithm::createHuffmanTree(frequencies);

        // Generate Huffman Codes
        std::unordered_map<char, std::string> huffmanCodes;
        atom::algorithm::generateHuffmanCodes(huffmanTree.get(), "",
                                              huffmanCodes);

        // Example text
        std::string text = "abcdef";

        // Compress Text
        std::string compressedText =
            atom::algorithm::compressText(text, huffmanCodes);

        std::cout << "Compressed Text: " << compressedText << std::endl;
    }

    {
        // Example frequency map
        std::unordered_map<char, int> frequencies = {
            {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}};

        // Create Huffman Tree
        auto huffmanTree = atom::algorithm::createHuffmanTree(frequencies);

        // Generate Huffman Codes
        std::unordered_map<char, std::string> huffmanCodes;
        atom::algorithm::generateHuffmanCodes(huffmanTree.get(), "",
                                              huffmanCodes);

        // Example text
        std::string text = "abcdef";

        // Compress Text
        std::string compressedText =
            atom::algorithm::compressText(text, huffmanCodes);

        // Decompress Text
        std::string decompressedText =
            atom::algorithm::decompressText(compressedText, huffmanTree.get());

        std::cout << "Decompressed Text: " << decompressedText << std::endl;
    }

    return 0;
}
