# 哈夫曼编码

## HuffmanNode 结构体

**简介：** 表示哈夫曼树中的一个节点。

- `char data`: 节点中存储的字符。
- `int frequency`: 节点的频率。
- `HuffmanNode *left`: 指向左子节点的指针。
- `HuffmanNode *right`: 指向右子节点的指针。

### 构造函数

- **参数:**
  - `data`: 要存储在节点中的字符。
  - `frequency`: 节点的频率。

## createHuffmanTree

**简介：** 创建一个哈夫曼树。

- **参数:**

  - `frequencies`: 字符频率表。

- **返回:**
  - `HuffmanNode*`: 指向哈夫曼树根节点的指针。

## generateHuffmanCodes

**简介：** 递归生成哈夫曼编码。

- **参数:**
  - `root`: 指向当前节点的指针。
  - `code`: 当前节点的编码。
  - `huffmanCodes`: 用于存储哈夫曼编码的映射。

## compressText

**简介：** 使用哈夫曼编码压缩文本。

- **参数:**

  - `text`: 要压缩的文本。
  - `huffmanCodes`: 包含哈夫曼编码的映射。

- **返回:**
  - `std::string`: 压缩后的文本。

## decompressText

**简介：** 使用哈夫曼编码解压文本。

- **参数:**

  - `compressedText`: 压缩后的文本。
  - `root`: 指向哈夫曼树根节点的指针。

- **返回:**
  - `std::string`: 解压后的文本。

### 示例用法

```cpp
#include <unordered_map>
#include <string>

// 定义字符频率
std::unordered_map<char, int> frequencies = {
    {'a', 10},
    {'b', 15},
    {'c', 30},
    {'d', 5},
    {'e', 20}
};

// 创建哈夫曼树
HuffmanNode *root = createHuffmanTree(frequencies);

// 生成哈夫曼编码
std::unordered_map<char, std::string> huffmanCodes;
generateHuffmanCodes(root, "", huffmanCodes);

// 使用哈夫曼编码压缩文本
std::string text = "abcde";
std::string compressedText = compressText(text, huffmanCodes);

// 使用哈夫曼编码解压文本
std::string decompressedText = decompressText(compressedText, root);
```
