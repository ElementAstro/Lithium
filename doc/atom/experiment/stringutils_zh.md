# StringUtils.h

## 简介

这个 C++ 头文件提供了实用函数，用于将各种数据类型转换为字符串，以便于输出和操作。这些函数处理不同类型的容器、映射、键值对和基本数据类型。

### 功能

1. 将基本数据类型和字符串转换为字符串。
2. 将容器（向量、列表等）转换为字符串。
3. 将键值对转换为字符串。
4. 使用自定义分隔符连接键值对。
5. 将命令行参数连接为单个字符串。
6. 生成数组的字符串表示。

### 函数

#### `toString`

```cpp
template <typename T>
auto toString(const T &value) -> std::enable_if_t<!is_map<T>::value && !is_container<T>::value, std::string>

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue)

template <typename Key, typename Value>
auto toString(const std::pair<Key, Value> &keyValue, const std::string &separator)

template <typename Container>
std::enable_if_t<is_map<Container>::value, std::string> toString(const Container &container)

template <typename Container>
auto toString(const Container &container) -> std::enable_if_t<is_container<Container>::value &&
                            !is_map<Container>::value &&
                            !is_string_type<typename Container::value_type>, std::string>

template <typename T>
std::string toString(const std::vector<T> &value)
```

#### `joinKeyValuePair`

```cpp
template <typename T>
std::enable_if_t<is_string_type<T>, std::string> joinKeyValuePair(const std::string &key, const T &value, const std::string &separator = "")

template <typename Key, typename Value>
std::string joinKeyValuePair(const std::pair<Key, Value> &keyValue, const std::string &separator = "")
```

#### `joinCommandLine`

```cpp
template <typename... Args>
[[nodiscard]] std::string joinCommandLine(const Args &...args)
```

#### `toStringArray`

```cpp
template <typename T>
auto toStringArray(const std::vector<T> &array)
```

### 示例用法

```cpp
#include <iostream>
#include "string_utils.h" // 包含头文件

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::map<std::string, int> ages = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}};

    // 将基本类型转换为字符串
    std::string numberAsString = toString(42);
    std::string text = toString("Hello, World!");

    // 将容器转换为字符串
    std::string numbersString = toString(numbers);
    std::string agesString = toString(ages);

    // 连接键值对
    std::string keyValueString = joinKeyValuePair("Name", "Alice", ": ");
    std::string pairString = joinKeyValuePair(std::make_pair("Age", 30), " = ");

    // 连接命令行参数
    std::string commandLine = joinCommandLine("ls", "-l", "-a");

    // 生成数组的字符串表示
    std::string arrayString = toStringArray(numbers);

    // 输出结果
    std::cout << numberAsString << std::endl;
    std::cout << text << std::endl;
    std::cout << numbersString << std::endl;
    std::cout << agesString << std::endl;
    std::cout << keyValueString << std::endl;
    std::cout << pairString << std::endl;
    std::cout << commandLine << std::endl;
    std::cout << arrayString << std::endl;

    return 0;
}
```

### 函数解释

- **`toString`**: 将各种类型转换为字符串。
- **`joinKeyValuePair`**: 使用自定义分隔符连接键值对。
- **`joinCommandLine`**: 将命令行参数连接为单个字符串。
- **`toStringArray`**: 生成数组的字符串表示。
