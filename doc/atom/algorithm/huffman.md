# Huffman Coding

## HuffmanNode Struct

**Brief:** Represents a node in a Huffman tree.

- `char data`: The character stored in the node.
- `int frequency`: The frequency of the node.
- `HuffmanNode *left`: Pointer to the left child node.
- `HuffmanNode *right`: Pointer to the right child node.

### Constructor

- **Parameters:**
  - `data`: The character to be stored in the node.
  - `frequency`: The frequency of the node.

## createHuffmanTree

**Brief:** Creates a Huffman tree.

- **Parameters:**

  - `frequencies`: Table of character frequencies.

- **Return:**
  - `HuffmanNode*`: Pointer to the root node of the Huffman tree.

## generateHuffmanCodes

**Brief:** Recursively generates Huffman codes.

- **Parameters:**
  - `root`: Pointer to the current node.
  - `code`: Current code for the node.
  - `huffmanCodes`: Map to store the Huffman codes.

## compressText

**Brief:** Compresses text using Huffman coding.

- **Parameters:**

  - `text`: Text to be compressed.
  - `huffmanCodes`: Map containing the Huffman codes.

- **Return:**
  - `std::string`: Compressed text.

## decompressText

**Brief:** Decompresses text using Huffman coding.

- **Parameters:**

  - `compressedText`: Compressed text.
  - `root`: Pointer to the root node of the Huffman tree.

- **Return:**
  - `std::string`: Decompressed text.

### Example Usage

```cpp
#include <unordered_map>
#include <string>

// Define character frequencies
std::unordered_map<char, int> frequencies = {
    {'a', 10},
    {'b', 15},
    {'c', 30},
    {'d', 5},
    {'e', 20}
};

// Create Huffman tree
HuffmanNode *root = createHuffmanTree(frequencies);

// Generate Huffman codes
std::unordered_map<char, std::string> huffmanCodes;
generateHuffmanCodes(root, "", huffmanCodes);

// Compress text using Huffman coding
std::string text = "abcde";
std::string compressedText = compressText(text, huffmanCodes);

// Decompress text using Huffman coding
std::string decompressedText = decompressText(compressedText, root);
```
