# Proxy Function Documentation

This document describes the usage of the `ProxyFunction` and `TimerProxyFunction` classes from the `atom::meta` namespace, which provide powerful function proxying capabilities.

## Table of Contents

1. [Introduction](#introduction)
2. [ProxyFunction Class](#proxyfunction-class)
3. [TimerProxyFunction Class](#timerproxyfunction-class)
4. [Usage Examples](#usage-examples)
5. [Helper Functions](#helper-functions)

## Introduction

The proxy function implementation allows you to wrap and call functions dynamically, providing additional functionality such as type-safe argument passing and execution timing.

## ProxyFunction Class

The `ProxyFunction` class is a template class that wraps a function or member function, allowing it to be called with a vector of `std::any` arguments or `FunctionParams`.

### Template Parameters

- `Func`: The type of the function to be wrapped.

### Constructor

```cpp
explicit ProxyFunction(Func&& func);
```

Creates a `ProxyFunction` instance wrapping the given function.

### Member Functions

```cpp
auto operator()(const std::vector<std::any>& args) -> std::any;
auto operator()(const FunctionParams& params) -> std::any;
```

These overloaded call operators allow you to invoke the wrapped function with either a vector of `std::any` arguments or `FunctionParams`.

## TimerProxyFunction Class

The `TimerProxyFunction` class extends the functionality of `ProxyFunction` by adding a timeout mechanism to the function execution.

### Template Parameters

- `Func`: The type of the function to be wrapped.

### Constructor

```cpp
explicit TimerProxyFunction(Func&& func);
```

Creates a `TimerProxyFunction` instance wrapping the given function.

### Member Functions

```cpp
auto operator()(const std::vector<std::any>& args, std::chrono::milliseconds timeout) -> std::any;
```

This call operator allows you to invoke the wrapped function with a vector of `std::any` arguments and a specified timeout.

## Usage Examples

Here are some examples demonstrating how to use the `ProxyFunction` and `TimerProxyFunction` classes:

### Example 1: Basic Usage of ProxyFunction

```cpp
#include "proxy.hpp"
#include <iostream>
#include <vector>

int add(int a, int b) {
    return a + b;
}

int main() {
    atom::meta::ProxyFunction proxy_add(add);

    std::vector<std::any> args = {std::any(5), std::any(3)};
    std::any result = proxy_add(args);

    std::cout << "Result: " << std::any_cast<int>(result) << std::endl;

    return 0;
}
```

### Example 2: Using ProxyFunction with Member Functions

```cpp
#include "proxy.hpp"
#include <iostream>
#include <vector>

class Calculator {
public:
    int multiply(int a, int b) const {
        return a * b;
    }
};

int main() {
    Calculator calc;
    atom::meta::ProxyFunction proxy_multiply(&Calculator::multiply);

    std::vector<std::any> args = {std::any(std::ref(calc)), std::any(4), std::any(7)};
    std::any result = proxy_multiply(args);

    std::cout << "Result: " << std::any_cast<int>(result) << std::endl;

    return 0;
}
```

### Example 3: Using TimerProxyFunction

```cpp
#include "proxy.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void long_running_function() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main() {
    atom::meta::TimerProxyFunction proxy_long_running(long_running_function);

    std::vector<std::any> args;
    try {
        proxy_long_running(args, std::chrono::seconds(3));
        std::cout << "Function completed within timeout." << std::endl;
    } catch (const atom::error::TimeoutException& e) {
        std::cout << "Function timed out: " << e.what() << std::endl;
    }

    return 0;
}
```

## Helper Functions

The `proxy.hpp` file also includes several helper functions for type casting:

```cpp
template <typename T>
auto anyCastRef(std::any& operand) -> T&&;

template <typename T>
auto anyCastRef(const std::any& operand) -> T&;

template <typename T>
auto anyCastVal(std::any& operand) -> T;

template <typename T>
auto anyCastVal(const std::any& operand) -> T;

template <typename T>
auto anyCastConstRef(const std::any& operand) -> const T&;

template <typename T>
auto anyCastHelper(std::any& operand) -> decltype(auto);

template <typename T>
auto anyCastHelper(const std::any& operand) -> decltype(auto);
```

These functions provide various ways to cast `std::any` objects to the desired types, handling references and const-ness correctly.

---
