# C++ 中的 Optional 类

## 简介

`Optional` 类提供了一种表示可选值的方式，该值可以包含或不包含类型为 `T` 的有效对象。它允许您安全地处理可能缺失的值，消除了空指针的需要。

## 类定义

```cpp
template <typename T>
class Optional {
private:
    alignas(T) unsigned char storage[sizeof(T)];
    bool hasValue;

public:
    // 构造函数
    Optional();
    Optional(const T &value);
    Optional(T &&value);
    Optional(const Optional &other);
    Optional(Optional &&other);

    // 析构函数
    ~Optional();

    // 赋值运算符
    Optional &operator=(const Optional &other);
    Optional &operator=(Optional &&other);

    // 成员函数
    T &operator*();
    const T &operator*() const;
    void reset();
    T &value();
    const T &value() const;
    explicit operator bool() const;
    bool operator==(const Optional &other) const;
    bool operator!=(const Optional &other) const;
    T value_or(const T &defaultValue) const;
};
```

## 使用示例

### 创建和使用 Optional 对象

```cpp
#include <iostream>
#include "Optional.h"

int main() {
    Optional<int> optInt;
    std::cout << "Optional 是否有值: " << static_cast<bool>(optInt) << std::endl; // 输出: 0 (false)

    Optional<std::string> optStr("Hello");
    std::cout << "Optional 是否有值: " << static_cast<bool>(optStr) << std::endl; // 输出: 1 (true)

    try {
        std::cout << "optStr 中的值: " << *optStr << std::endl; // 输出: Hello
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    Optional<std::vector<int>> optVec(std::vector<int>{1, 2, 3});
    std::cout << "optVec 中的值: ";
    for (const auto &num : optVec.value()) {
        std::cout << num << " ";
    }
    std::cout << std::endl; // 输出: 1 2 3

    return 0;
}
```

## 功能

- `Optional` 类允许您可选地存储类型为 `T` 的值。
- 它通过操作符如 `*` 和 `value()` 提供安全访问包含的值。
- 您可以使用布尔转换运算符检查 `Optional` 是否包含值。
- `reset()` 函数清除存储的值。
- 提供了比较运算符 `==` 和 `!=` 用于比较 `Optional` 对象。
- `value_or()` 函数返回包含的值或默认值（如果不存在值）。

## 结论

C++ 中的 `Optional` 类提供了一种更安全、更表达性的处理可选值的方式，避免了空指针的问题。通过遵循使用示例并了解类函数，您可以利用 `Optional` 提升代码的健壮性。
