# Huffman Encoding Documentation

## Overview

The `huffman.hpp` file provides a simple implementation of Huffman encoding, a data compression algorithm. It includes structures and functions for creating a Huffman tree, generating Huffman codes, compressing text, and decompressing encoded text.

## Namespace

All structures and functions are defined within the `atom::algorithm` namespace.

```cpp
namespace atom::algorithm {
    // ...
}
```

## Structures

### HuffmanNode

```cpp
struct HuffmanNode {
    char data;
    int frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;

    explicit HuffmanNode(char data, int frequency);
};
```

Represents a node in the Huffman tree.

- **Members:**

  - `data`: Character stored in this node (used only in leaf nodes).
  - `frequency`: Frequency of the character or sum of frequencies for internal nodes.
  - `left`: Pointer to the left child node.
  - `right`: Pointer to the right child node.

- **Constructor:**
  - `HuffmanNode(char data, int frequency)`: Constructs a new Huffman Node with the given character and frequency.

## Functions

### createHuffmanTree

```cpp
[[nodiscard]] auto createHuffmanTree(
    const std::unordered_map<char, int>& frequencies)
    -> std::shared_ptr<HuffmanNode>;
```

Creates a Huffman tree based on the frequency of characters.

- **Parameters:**
  - `frequencies`: A map of characters and their corresponding frequencies.
- **Returns:** A shared pointer to the root of the Huffman tree.
- **Description:** This function builds a Huffman tree using the frequencies of characters in the input text. It employs a priority queue to build the tree from the bottom up by merging the two least frequent nodes until only one node remains, which becomes the root.

### generateHuffmanCodes

```cpp
void generateHuffmanCodes(const HuffmanNode* root, const std::string& code,
                          std::unordered_map<char, std::string>& huffmanCodes);
```

Generates Huffman codes for each character from the Huffman tree.

- **Parameters:**
  - `root`: Pointer to the root node of the Huffman tree.
  - `code`: Current Huffman code generated during the traversal.
  - `huffmanCodes`: A reference to a map where the character and its corresponding Huffman code will be stored.
- **Description:** This function recursively traverses the Huffman tree and assigns a binary code to each character. These codes are derived from the path taken to reach the character: left child gives '0' and right child gives '1'.

### compressText

```cpp
[[nodiscard]] auto compressText(
    std::string_view TEXT,
    const std::unordered_map<char, std::string>& huffmanCodes) -> std::string;
```

Compresses text using Huffman codes.

- **Parameters:**
  - `TEXT`: The original text to compress.
  - `huffmanCodes`: The map of characters to their corresponding Huffman codes.
- **Returns:** A string representing the compressed text.
- **Description:** This function converts a string of text into a string of binary codes based on the Huffman codes provided. Each character in the input text is replaced by its corresponding Huffman code.

### decompressText

```cpp
[[nodiscard]] auto decompressText(std::string_view COMPRESSED_TEXT,
                                  const HuffmanNode* root) -> std::string;
```

Decompresses Huffman encoded text back to its original form.

- **Parameters:**
  - `COMPRESSED_TEXT`: The Huffman encoded text.
  - `root`: Pointer to the root of the Huffman tree.
- **Returns:** The original decompressed text.
- **Description:** This function decodes a string of binary codes back into the original text using the provided Huffman tree. It traverses the Huffman tree from the root to the leaf nodes based on the binary string, reconstructing the original text.

## Usage Example

Here's an example demonstrating how to use the Huffman encoding functions:

```cpp
#include "huffman.hpp"
#include <iostream>
#include <unordered_map>

int main() {
    // Sample text to compress
    std::string text = "hello world";

    // Step 1: Calculate character frequencies
    std::unordered_map<char, int> frequencies;
    for (char c : text) {
        frequencies[c]++;
    }

    // Step 2: Create Huffman tree
    auto root = atom::algorithm::createHuffmanTree(frequencies);

    // Step 3: Generate Huffman codes
    std::unordered_map<char, std::string> huffmanCodes;
    atom::algorithm::generateHuffmanCodes(root.get(), "", huffmanCodes);

    // Step 4: Compress the text
    std::string compressedText = atom::algorithm::compressText(text, huffmanCodes);

    std::cout << "Original text: " << text << std::endl;
    std::cout << "Compressed text: " << compressedText << std::endl;

    // Step 5: Decompress the text
    std::string decompressedText = atom::algorithm::decompressText(compressedText, root.get());

    std::cout << "Decompressed text: " << decompressedText << std::endl;

    return 0;
}
```

This example demonstrates the full process of Huffman encoding:

1. Calculating character frequencies
2. Creating the Huffman tree
3. Generating Huffman codes
4. Compressing the text
5. Decompressing the text

## Best Practices

1. Ensure that the input text for compression is not empty to avoid undefined behavior.
2. When working with large texts, consider using file I/O instead of storing the entire text in memory.
3. For efficiency in real-world applications, consider implementing bit-level operations for the compressed data instead of using strings of '0's and '1's.
4. Always keep the Huffman tree or a serialized version of it along with the compressed data, as it's necessary for decompression.

## Notes

- This implementation uses `std::shared_ptr` for memory management of the Huffman tree nodes.
- The compression effectiveness depends on the frequency distribution of characters in the input text. It works best when there's a significant variation in character frequencies.
- While this implementation provides a good understanding of Huffman encoding, for production use, consider using established libraries that have been optimized and thoroughly tested.
