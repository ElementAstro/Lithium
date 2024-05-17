# 哈希实用工具

**作者:** Max Qian
**日期:** 2024 年 3 月 28 日
**版本:** 1.0

---

## 概述

Atom 算法哈希库是一个用 C++实现的哈希算法集合，为开发人员提供了一种多功能且高效的方式来计算各种数据类型的哈希值。它提供了计算单个值、向量、元组和数组的哈希值的功能。

## 特点

- **通用哈希:** 该库支持满足`Hashable`概念的任何数据类型的哈希，包括具有定义的`std::hash`专门化的内置类型和用户定义类型。
- **容器支持:** 提供了对`std::vector`、`std::tuple`和`std::array`的哈希函数，允许用户计算集合元素的哈希值。

- **可定制哈希:** 用户可以通过为其自定义类型提供`std::hash`专门化轻松扩展库，实现与现有代码库的无缝集成。

## 概念

### Hashable 概念

`Hashable`概念指定类型必须可使用`std::hash`进行哈希。任何为`std::hash`专门化的类型都满足此概念。

### computeHash 函数

该库提供了几个重载的`computeHash`函数，每个函数都专门用于计算不同类型输入的哈希值：

- `computeHash(const T& value)`: 计算类型`T`的单个值的哈希值。
- `computeHash(const std::vector<T>& values)`: 计算类型`T`元素向量的哈希值。

- `computeHash(const std::tuple<Ts...>& tuple)`: 计算元素元组的哈希值，其中`Ts...`表示元组元素的类型。

- `computeHash(const std::array<T, N>& array)`: 计算类型`T`和大小`N`的数组的哈希值。

## 用法

要使用 Atom 算法哈希库，在您的 C++项目中包含`hash.hpp`头文件，并确保使用 C++17 或更高版本进行编译。

```cpp
#include "hash.hpp"
#include <iostream>
#include <vector>
#include <tuple>

int main() {
    std::vector<int> values = {1, 2, 3, 4, 5};
    std::size_t hashValue = computeHash(values);
    std::cout << "向量的哈希值: " << hashValue << std::endl;

    std::tuple<int, double, char> tuple = {1, 3.14, 'a'};
    hashValue = computeHash(tuple);
    std::cout << "元组的哈希值: " << hashValue << std::endl;

    return 0;
}
```
