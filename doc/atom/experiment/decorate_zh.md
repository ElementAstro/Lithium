# C++ 函数装饰器

## 简介

这是一个用于函数装饰的 C++ 库，提供了一种在不直接修改函数代码的情况下为函数添加额外行为的方法。它允许向现有函数添加钩子、条件检查和循环等功能。

## 使用方法

### 基本装饰

要使用函数装饰器，首先包含必要的头文件并定义您的函数。然后，使用 `make_decorator` 函数创建一个装饰器，通过 `with_hooks` 方法添加钩子或其他行为。

```cpp
#include "function_decorator.h"

void myFunction(int arg) {
    std::cout << "执行带参数 " << arg << " 的函数" << std::endl;
}

int main() {
    auto decoratedFunc = make_decorator(myFunction).with_hooks(
        []() { std::cout << "在函数执行前" << std::endl; },
        [](int result) {
            std::cout << "在函数执行后，结果为 " << result
                      << std::endl;
        },
        [](long long timeTaken) {
            std::cout << "函数执行时间为 " << timeTaken << " 微秒"
                      << std::endl;
        });

    decoratedFunc(42);

    return 0;
}
```

### 循环装饰

您还可以使用 `make_loop_decorator` 函数创建一个循环装饰器，在循环中多次执行该函数。

```cpp
auto loopDecoratedFunc = make_loop_decorator(myFunction);
loopDecoratedFunc(3, 42);  // 以参数 42 运行 myFunction 函数 3 次
```

### 条件检查装饰

可以创建条件检查装饰器，根据条件在满足时有选择性地执行函数。

```cpp
auto conditionDecoratedFunc = make_condition_check_decorator(myFunction);
conditionDecoratedFunc([]() { return true; }, 42);  // 如果条件为真，则执行 myFunction
```

### 链式装饰器

可以使用 `DecorateStepper` 类将多个装饰器链接在一起，创建一个应用于函数的装饰器序列。

```cpp
DecorateStepper<int, int> stepper(myFunction);
stepper.addDecorator<LoopDecorator>(3); // 循环 3 次
stepper.addDecorator<ConditionCheckDecorator>([]() { return true; }); // 有条件地执行
stepper.execute(42);  // 执行装饰后的函数
```

## 结论

函数装饰器库提供了一种灵活的方式来为现有函数添加功能，实现对行为的轻松扩展和修改，而无需直接修改原始函数代码。
