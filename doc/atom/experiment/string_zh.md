# 超级增强字符串类文档

`String` 类是一个具有各种操作和功能的超级增强字符串类。以下是其特性和函数的详细文档。

## 构造函数

### String()

- **简介**：默认构造函数。

### String(const char \*str)

- **简介**：接受 C 风格字符串作为输入的构造函数。

### String(const std::string &str)

- **简介**：接受 `std::string` 作为输入的构造函数。

### String(const String &other)

- **简介**：拷贝构造函数。

## 成员函数

### operator=(const String &other)

- **简介**：赋值操作符。

### operator==(const String &other) const

- **简介**：相等比较。

### operator!=(const String &other) const

- **简介**：不等比较。

### operator bool() const

- **简介**：检查字符串是否非空。

### operator<(const String &other) const

- **简介**：小于比较。

### operator>(const String &other) const

- **简介**：大于比较。

### operator<=(const String &other) const

- **简介**：小于等于比较。

### operator>=(const String &other) const

- **简介**：大于等于比较。

### operator+=(const String &other)

- **简介**：与另一个 `String` 进行连接。

### operator+=(const char \*str)

- **简介**：与 C 风格字符串进行连接。

### operator+=(char c)

- **简介**：与单个字符进行连接。

### toCharArray() const

- **简介**：获取 C 风格字符串。

### length() const

- **简介**：获取字符串长度。

### substring(size_t pos, size_t len) const

- **简介**：从字符串中获取子串。

### find(const String &str, size_t pos) const

- **简介**：在字符串中查找子字符串。

### replace(const String &oldStr, const String &newStr)

- **简介**：用另一个子字符串替换子字符串的出现。

### toUpperCase() const

- **简介**：将字符串转换为大写。

### toLowerCase() const

- **简介**：将字符串转换为小写。

### split(const String &delimiter) const

- **简介**：使用分隔符拆分字符串。

### join(const std::vector<`String`> &strings, const String &separator)

- **简介**：使用分隔符连接字符串向量。

### replaceAll(const String &oldStr, const String &newStr)

- **简介**：替换所有子字符串的出现。

### insertChar(size_t pos, char c)

- **简介**：在指定位置插入字符。

### deleteChar(size_t pos)

- **简介**：删除指定位置的字符。

### reverse() const

- **简介**：获取字符串的反转。

### equalsIgnoreCase(const String &other) const

- **简介**：不区分大小写的相等比较。

### indexOf(const String &subStr, size_t startPos) const

- **简介**：从指定位置开始获取子字符串的索引。

### trim()

- **简介**：从字符串中删除前导和尾随空格。

### startsWith(const String &prefix) const

- **简介**：检查字符串是否以特定前缀开头。

### endsWith(const String &suffix) const

- **简介**：检查字符串是否以特定后缀结尾。

### escape() const

- **简介**：对字符串中的特殊字符进行转义。

### unescape() const

- **简介**：对字符串中的转义字符进行反转义。

### toInt() const

- **简介**：将字符串转换为整数。

### toFloat() const

- **简介**：将字符串转换为浮点数。

### format(const char \*format, ...)

- **简介**：使用 printf 风格格式化字符串。

## 非成员函数

### operator+(const String &lhs, const String &rhs)

- **简介**：连接两个字符串。

## 静态成员

### npos

- **简介**：表示 '未找到' 或 '无效' 位置的常量。

## 使用示例

```cpp
#include "String.h"

int main() {
    String s1("Hello");
    String s2("World");

    // 连接
    String result = s1 + " " + s2;
    std::cout << result.toCharArray() << std::endl;

    // 转换为大写
    String upper = result.toUpperCase();
    std::cout << upper.toCharArray() << std::endl;

    // 查找操作
    size_t pos = result.find("World", 0);
    std::cout << "Position of 'World': " << pos << std::endl;

    return 0;
}
```
