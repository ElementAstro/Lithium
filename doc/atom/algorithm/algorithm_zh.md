# 算法库文档

## 概述

该库是一个 C++实现的算法集合，包括 Knuth-Morris-Pratt (KMP)字符串搜索算法、Boyer-Moore 字符串搜索算法和一个通用的布隆过滤器（Bloom Filter）数据结构。

## 命名空间

### `atom::algorithm`

此命名空间包含了库中提供的所有算法实现的类和函数。

## 类

### `KMP`

实现了 Knuth-Morris-Pratt (KMP)字符串搜索算法。

#### 构造函数

- `explicit KMP(std::string_view pattern)`

  使用给定的模式构造一个`KMP`对象。

  **参数:**

  - `pattern` - 要在文本中搜索的模式。

#### 公共方法

- `[[nodiscard]] auto search(std::string_view text) const -> std::vector<int>`

  使用 KMP 算法在给定文本中搜索模式的出现位置。

  **参数:**

  - `text` - 要搜索的文本。

  **返回:**

  - `std::vector<int>` - 包含模式在文本中起始位置的向量。

- `void setPattern(std::string_view pattern)`

  设置新的搜索模式。

  **参数:**

  - `pattern` - 要搜索的新模式。

#### 私有方法

- `auto computeFailureFunction(std::string_view pattern) -> std::vector<int>`

  计算给定模式的失败函数（部分匹配表）。

  **参数:**

  - `pattern` - 要计算失败函数的模式。

  **返回:**

  - `std::vector<int>` - 计算出的失败函数（部分匹配表）。

#### 数据成员

- `std::string pattern_`

  要搜索的模式。

- `std::vector<int> failure_`

  模式的失败函数（部分匹配表）。

### `BloomFilter<N>`

实现了布隆过滤器（Bloom Filter）数据结构。

#### 模板参数

- `N` - 布隆过滤器的大小（位数）。

#### 构造函数

- `explicit BloomFilter(std::size_t num_hash_functions)`

  使用指定数量的哈希函数构造一个新的`BloomFilter`对象。

  **参数:**

  - `num_hash_functions` - 用于布隆过滤器的哈希函数数量。

#### 公共方法

- `void insert(std::string_view element)`

  将一个元素插入到布隆过滤器中。

  **参数:**

  - `element` - 要插入的元素。

- `[[nodiscard]] auto contains(std::string_view element) const -> bool`

  检查布隆过滤器中是否可能包含某个元素。

  **参数:**

  - `element` - 要检查的元素。

  **返回:**

  - `bool` - 如果元素可能存在则返回`true`，否则返回`false`。

#### 私有方法

- `auto hash(std::string_view element, std::size_t seed) const -> std::size_t`

  使用特定的种子计算元素的哈希值。

  **参数:**

  - `element` - 要哈希的元素。
  - `seed` - 哈希函数的种子值。

  **返回:**

  - `std::size_t` - 元素的哈希值。

#### 数据成员

- `std::bitset<N> m_bits_`

  代表布隆过滤器的位集。

- `std::size_t m_num_hash_functions_`

  使用的哈希函数数量。

### `BoyerMoore`

实现了 Boyer-Moore 字符串搜索算法。

#### 构造函数

- `explicit BoyerMoore(std::string_view pattern)`

  使用给定的模式构造一个`BoyerMoore`对象。

  **参数:**

  - `pattern` - 要在文本中搜索的模式。

#### 公共方法

- `auto search(std::string_view text) const -> std::vector<int>`

  使用 Boyer-Moore 算法在给定文本中搜索模式的出现位置。

  **参数:**

  - `text` - 要搜索的文本。

  **返回:**

  - `std::vector<int>` - 包含模式在文本中起始位置的向量。

- `void setPattern(std::string_view pattern)`

  设置新的搜索模式。

  **参数:**

  - `pattern` - 要搜索的新模式。

#### 私有方法

- `void computeBadCharacterShift()`

  计算当前模式的坏字符偏移表。

- `void computeGoodSuffixShift()`

  计算当前模式的好后缀偏移表。

#### 数据成员

- `std::string pattern_`

  要搜索的模式。

- `std::unordered_map<char, int> bad_char_shift_`

  坏字符偏移表。

- `std::vector<int> good_suffix_shift_`

  好后缀偏移表。
