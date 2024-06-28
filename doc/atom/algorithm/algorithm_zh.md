# Algorithm Library

这是一个用于C++的算法库,包含了几种常用的字符串搜索和相似度估计算法。

## 命名空间

所有的算法类都位于`atom::algorithm`命名空间下。

## 类

### KMP

KMP类实现了Knuth-Morris-Pratt字符串搜索算法。

#### 构造函数

- `explicit KMP(std::string_view pattern)`: 使用指定的模式字符串构造一个新的KMP对象。

#### 成员函数

- `auto search(std::string_view text) -> std::vector<int>`: 在给定文本中搜索模式的出现位置,返回所有出现位置的起始索引。
- `void setPattern(std::string_view pattern)`: 设置一个新的搜索模式。

#### 私有成员函数

- `auto computeFailureFunction(std::string_view pattern) -> std::vector<int>`: 计算指定模式的失败函数。

#### 私有成员变量

- `std::string pattern_`: 要搜索的模式字符串。
- `std::vector<int> failure_`: 当前模式的失败函数。

### MinHash

MinHash类实现了MinHash算法,用于估计集合之间的Jaccard相似度。

#### 构造函数

- `explicit MinHash(int num_hash_functions)`: 使用指定数量的哈希函数构造一个新的MinHash对象。

#### 成员函数

- `auto computeSignature(const std::unordered_set<std::string>& set) -> std::vector<unsigned long long>`: 计算给定集合的MinHash签名。
- `auto estimateSimilarity(const std::vector<unsigned long long>& signature1, const std::vector<unsigned long long>& signature2) const -> double`: 使用MinHash签名估计两个集合之间的Jaccard相似度。

#### 私有成员函数

- `unsigned long long hash(const std::string& element, int index)`: 使用特定的哈希函数计算元素的哈希值。

#### 私有成员变量

- `int m_num_hash_functions_`: 用于MinHash的哈希函数数量。
- `std::vector<unsigned long long> m_coefficients_a_`: 哈希函数的系数'a'。
- `std::vector<unsigned long long> m_coefficients_b_`: 哈希函数的系数'b'。

### BloomFilter

BloomFilter类实现了布隆过滤器数据结构。

#### 模板参数

- `N`: 布隆过滤器的大小(比特数)。

#### 构造函数

- `explicit BloomFilter(std::size_t num_hash_functions)`: 使用指定数量的哈希函数构造一个新的BloomFilter对象。

#### 成员函数

- `void insert(std::string_view element)`: 将元素插入布隆过滤器。
- `bool contains(std::string_view element) const`: 检查元素是否可能存在于布隆过滤器中。

#### 私有成员函数

- `auto hash(std::string_view element, std::size_t seed) const -> std::size_t`: 使用特定的种子值计算元素的哈希值。

#### 私有成员变量

- `std::bitset<N> m_bits_`: 表示布隆过滤器的比特集。
- `std::size_t m_num_hash_functions_`: 使用的哈希函数数量。

### BoyerMoore

BoyerMoore类实现了Boyer-Moore字符串搜索算法。

#### 构造函数

- `explicit BoyerMoore(std::string_view pattern)`: 使用指定的模式字符串构造一个新的BoyerMoore对象。

#### 成员函数

- `auto search(std::string_view text) -> std::vector<int>`: 在给定文本中搜索模式的出现位置,返回所有出现位置的起始索引。
- `void setPattern(std::string_view pattern)`: 为BoyerMoore对象设置一个新的搜索模式。

#### 私有成员函数

- `void computeBadCharacterShift()`: 计算模式的坏字符移位表。
- `void computeGoodSuffixShift()`: 计算模式的好后缀移位表。

#### 私有成员变量

- `std::string pattern_`: 要搜索的模式字符串。
- `std::unordered_map<char, int> bad_char_shift_`: 坏字符移位表。
- `std::vector<int> good_suffix_shift_`: 好后缀移位表。
