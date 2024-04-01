# MD5 类

MD5 类提供了使用 MD5 算法计算输入数据的 MD5 哈希的功能。

## 成员函数

### 公有方法

#### `static std::string encrypt(const std::string &input)`

使用 MD5 算法加密输入字符串，并返回输入字符串的 MD5 哈希。

### 私有方法

#### `void init()`

初始化内部变量和用于 MD5 计算的缓冲区。

#### `void update(const std::string &input)`

使用额外的输入数据更新 MD5 计算。

#### `std::string finalize()`

完成 MD5 计算并返回迄今为止提供的所有输入数据的哈希结果。

#### `void processBlock(const uint8_t *block)`

处理 64 字节的输入数据块。

### 静态辅助函数

#### `static uint32_t F(uint32_t x, uint32_t y, uint32_t z)`

MD5 算法中的 F 函数。

#### `static uint32_t G(uint32_t x, uint32_t y, uint32_t z)`

MD5 算法中的 G 函数。

#### `static uint32_t H(uint32_t x, uint32_t y, uint32_t z)`

MD5 算法中的 H 函数。

#### `static uint32_t I(uint32_t x, uint32_t y, uint32_t z)`

MD5 算法中的 I 函数。

#### `static uint32_t leftRotate(uint32_t x, uint32_t n)`

对输入执行左旋转操作。

#### `static uint32_t reverseBytes(uint32_t x)`

颠倒输入中的字节。

## 成员变量

- `uint32_t _a, _b, _c, _d`: MD5 计算的内部状态变量。
- `uint64_t _count`: 输入位的总计数。
- `std::vector<uint8_t> _buffer`: 输入数据的缓冲区。

## 示例用法

```cpp
#include "MD5.h"
#include <iostream>

int main() {
    std::string input = "Hello, World!";
    std::string md5Hash = MD5::encrypt(input);
    std::cout << "MD5 哈希值: " << md5Hash << std::endl;
    return 0;
}
```
