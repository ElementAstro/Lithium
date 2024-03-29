# Delayed Function Invocation in C++

## Introduction

This C++ library provides a way to delay the invocation of a function with arguments. It supports delayed execution of any callable object with arguments, allowing for flexibility in function invocation.

## Usage

### Requirements

For C++17 and above, the library uses `std::apply` to delay the invocation of the function. For C++14 and below, it provides an alternative implementation using template metaprogramming techniques.

### Basic Delayed Invocation

To delay the invocation of a function with arguments, use the `delay_invoke` function template as shown below:

```cpp
#include <iostream>
#include "invoke.hpp"

void myFunction(int arg1, int arg2) {
    std::cout << "Executing function with args: " << arg1 << ", " << arg2 << std::endl;
}

int main() {
    auto delayedFunc = delay_invoke<void>(myFunction, 42, 100);
    // The function myFunction will be executed with args 42 and 100 when delayedFunc is called
    delayedFunc();

    return 0;
}
```

### Explanation

The `delay_invoke` function delays the invocation of `myFunction` with arguments `42` and `100`. When `delayedFunc` is called, `myFunction` is executed with the provided arguments.

## Conclusion

The delayed function invocation library in C++ offers a convenient way to postpone the execution of functions with arguments. It enhances code modularity and flexibility by allowing functions to be invoked at a later time with specified parameters.
