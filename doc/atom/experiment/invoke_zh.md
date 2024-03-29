# 在 C++ 中延迟函数调用

## 简介

这是一个提供延迟调用带参数函数的 C++ 库，支持延迟执行任何带参数的可调用对象，为函数调用提供了灵活性。

## 使用方法

### 要求

对于 C++17 及更高版本，该库使用 `std::apply` 来延迟函数的调用。对于 C++14 及更低版本，它提供了一种使用模板元编程技术的替代实现。

### 基本延迟调用

要延迟调用带参数的函数，可以使用以下示例中的 `delay_invoke` 函数模板：

```cpp
#include <iostream>
#include "invoke.hpp"

void myFunction(int arg1, int arg2) {
    std::cout << "执行带参数的函数：" << arg1 << ", " << arg2 << std::endl;
}

int main() {
    auto delayedFunc = delay_invoke<void>(myFunction, 42, 100);
    // 当调用 delayedFunc 时，函数 myFunction 将以参数 42 和 100 执行
    delayedFunc();

    return 0;
}
```

### 解释

`delay_invoke` 函数延迟调用了 `myFunction`，并传入参数 `42` 和 `100`。当调用 `delayedFunc` 时，`myFunction` 将以提供的参数执行。

## 结论

C++ 中的延迟函数调用库为延迟执行带参数的函数提供了便利的方式。它通过允许在稍后的时间以指定的参数调用函数，增强了代码的模块化和灵活性。
