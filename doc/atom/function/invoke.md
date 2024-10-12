# Invoke.hpp Documentation

## Overview

The `invoke.hpp` file provides a set of utility functions for function invocation, including delayed invocation, safe calling, and exception handling. These utilities are designed to work with C++11 and C++17, offering flexible and safe ways to work with callable objects.

## Table of Contents

1. [Concepts](#concepts)
2. [Function Delay Invocation](#function-delay-invocation)
3. [Member Function Delay Invocation](#member-function-delay-invocation)
4. [Safe Function Calling](#safe-function-calling)
5. [Usage Examples](#usage-examples)
6. [Notes and Considerations](#notes-and-considerations)

## Concepts

### Invocable

```cpp
template <typename F, typename... Args>
concept Invocable = std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>;
```

This concept checks if a function type `F` is invocable with the given argument types `Args`.

## Function Delay Invocation

### delayInvoke

```cpp
template <typename F, typename... Args>
    requires Invocable<F, Args...>
auto delayInvoke(F &&func, Args &&...args);
```

Delays the invocation of a function with given arguments. Returns a lambda that, when called, invokes the function with the given arguments.

## Member Function Delay Invocation

### delayMemInvoke

```cpp
template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...), T *obj);

template <typename R, typename T, typename... Args>
auto delayMemInvoke(R (T::*func)(Args...) const, const T *obj);
```

Delays the invocation of a member function (both non-const and const versions) with given arguments.

### delayStaticMemInvoke

```cpp
template <typename R, typename T, typename... Args>
auto delayStaticMemInvoke(R (*func)(Args...), T *obj);
```

Delays the invocation of a static member function with given arguments.

### delayMemberVarInvoke

```cpp
template <typename T, typename M>
auto delayMemberVarInvoke(M T::*memberVar, T *obj);
```

Delays the access of a member variable.

## Safe Function Calling

### safeCall

```cpp
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safeCall(Func &&func, Args &&...args);
```

Safely calls a function with given arguments, catching any exceptions. Returns the result of the function call, or a default-constructed value if an exception occurs.

### safeTryCatch

```cpp
template <typename F, typename... Args>
    requires std::is_invocable_v<std::decay_t<F>, std::decay_t<Args>...>
auto safeTryCatch(F &&func, Args &&...args);
```

Safely tries to call a function with given arguments, catching any exceptions. Returns a variant containing either the result of the function call or an exception pointer.

### safeTryCatchOrDefault

```cpp
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safeTryCatchOrDefault(
    Func &&func,
    std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...> default_value,
    Args &&...args);
```

Safely tries to call a function with given arguments, returning a default value if an exception occurs.

### safeTryCatchWithCustomHandler

```cpp
template <typename Func, typename... Args>
    requires Invocable<Func, Args...>
auto safeTryCatchWithCustomHandler(
    Func &&func, const std::function<void(std::exception_ptr)> &handler,
    Args &&...args);
```

Safely tries to call a function with given arguments, using a custom handler if an exception occurs.

## Usage Examples

### Delayed Invocation

```cpp
#include "invoke.hpp"
#include <iostream>

void printSum(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
}

int main() {
    auto delayedPrintSum = delayInvoke(printSum, 5, 7);
    // ... some other operations ...
    delayedPrintSum(); // Prints: Sum: 12
    return 0;
}
```

### Member Function Delay Invocation

```cpp
#include "invoke.hpp"
#include <iostream>

class Calculator {
public:
    int add(int a, int b) const { return a + b; }
};

int main() {
    Calculator calc;
    auto delayedAdd = delayMemInvoke(&Calculator::add, &calc);
    std::cout << "Result: " << delayedAdd(10, 20) << std::endl; // Prints: Result: 30
    return 0;
}
```

### Safe Function Calling

```cpp
#include "invoke.hpp"
#include <iostream>
#include <stdexcept>

int divideNumbers(int a, int b) {
    if (b == 0) throw std::runtime_error("Division by zero");
    return a / b;
}

int main() {
    auto result1 = safeCall(divideNumbers, 10, 2);
    std::cout << "Result 1: " << result1 << std::endl; // Prints: Result 1: 5

    auto result2 = safeCall(divideNumbers, 10, 0);
    std::cout << "Result 2: " << result2 << std::endl; // Prints: Result 2: 0 (default int value)

    auto result3 = safeTryCatchOrDefault(divideNumbers, -1, 10, 0);
    std::cout << "Result 3: " << result3 << std::endl; // Prints: Result 3: -1

    return 0;
}
```

## Notes and Considerations

1. The `Invocable` concept and some functions use C++20 features. Ensure your compiler supports C++20 when using these features.
2. The `safeCall` function returns a default-constructed value for the return type if an exception occurs. This might not be suitable for all types, especially those without a default constructor.
3. The `safeTryCatch` function returns a `std::variant` which allows you to check whether the call was successful or resulted in an exception.
4. When using `safeTryCatchWithCustomHandler`, remember that if the handler doesn't rethrow the exception and the return type isn't default-constructible, the function will throw.
5. These utilities provide a way to handle exceptions and delay function invocations, which can be useful in many scenarios, including asynchronous programming and error handling.
6. Always consider the performance implications of using these utilities, especially in performance-critical code paths.

Remember to include the necessary headers and compile with C++17 or later support when using this library.
