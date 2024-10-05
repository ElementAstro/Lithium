# ArgsView Class Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Header File](#header-file)
3. [Class Definition](#class-definition)
4. [Member Functions](#member-functions)
5. [Non-Member Functions](#non-member-functions)
6. [Operator Overloads](#operator-overloads)
7. [Utility Functions](#utility-functions)
8. [Usage Examples](#usage-examples)
9. [Best Practices and Notes](#best-practices-and-notes)

## Introduction

The `ArgsView` class is a C++20 template class that provides a view over a set of arguments. It allows for efficient manipulation and transformation of argument lists without copying the underlying data.

## Header File

```cpp
#include "argsview.hpp"
```

## Class Definition

```cpp
template <typename... Args>
class ArgsView {
    // ... (member function declarations)
private:
    std::tuple<Args...> args_;
};
```

## Member Functions

### Constructors

1. Basic Constructor

   ```cpp
   constexpr explicit ArgsView(Args&&... args) noexcept;
   ```

   Constructs an `ArgsView` from a set of arguments.

2. Tuple Constructor

   ```cpp
   template <typename... OtherArgs>
   constexpr explicit ArgsView(const std::tuple<OtherArgs...>& other_tuple);
   ```

   Constructs an `ArgsView` from a tuple.

3. Copy Constructor from Another ArgsView
   ```cpp
   template <typename... OtherArgs>
   constexpr explicit ArgsView(ArgsView<OtherArgs...> other_args_view);
   ```
   Constructs an `ArgsView` from another `ArgsView`.

### Accessor Methods

1. `get`

   ```cpp
   template <std::size_t I>
   constexpr decltype(auto) get() const noexcept;
   ```

   Gets the argument at the specified index.

2. `size`

   ```cpp
   [[nodiscard]] constexpr std::size_t size() const noexcept;
   ```

   Returns the number of arguments.

3. `empty`
   ```cpp
   [[nodiscard]] constexpr bool empty() const noexcept;
   ```
   Checks if there are no arguments.

### Iteration and Transformation

1. `forEach`

   ```cpp
   template <typename Func>
   constexpr void forEach(Func&& func) const;
   ```

   Applies a function to each argument.

2. `transform`

   ```cpp
   template <typename F>
   auto transform(F&& f) const;
   ```

   Transforms the arguments using a function.

3. `accumulate`

   ```cpp
   template <typename Func, typename Init>
   constexpr auto accumulate(Func&& func, Init init) const;
   ```

   Accumulates the arguments using a function and an initial value.

4. `apply`
   ```cpp
   template <typename Func>
   constexpr auto apply(Func&& func) const;
   ```
   Applies a function to the arguments.

### Conversion

1. `toTuple`
   ```cpp
   std::tuple<Args...> toTuple() const;
   ```
   Converts the `ArgsView` to a tuple.

### Assignment Operators

1. Tuple Assignment

   ```cpp
   template <typename... OtherArgs>
   constexpr auto operator=(const std::tuple<OtherArgs...>& other_tuple) -> ArgsView&;
   ```

   Assigns the arguments from a tuple.

2. ArgsView Assignment
   ```cpp
   template <typename... OtherArgs>
   constexpr auto operator=(ArgsView<OtherArgs...> other_args_view) -> ArgsView&;
   ```
   Assigns the arguments from another `ArgsView`.

## Non-Member Functions

1. `makeArgsView`

   ```cpp
   template <typename... Args>
   constexpr auto makeArgsView(Args&&... args) -> ArgsViewT<Args...>;
   ```

   Creates an `ArgsView` from the given arguments.

2. `get`

   ```cpp
   template <std::size_t I, typename... Args>
   constexpr auto get(ArgsView<Args...> args_view) -> decltype(auto);
   ```

   Gets the argument at the specified index in an `ArgsView`.

3. `apply`

   ```cpp
   template <typename Func, typename... Args>
   constexpr auto apply(Func&& func, ArgsViewT<Args...> args_view);
   ```

   Applies a function to the arguments in an `ArgsView`.

4. `forEach`

   ```cpp
   template <typename Func, typename... Args>
   constexpr void forEach(Func&& func, ArgsView<Args...> args_view);
   ```

   Applies a function to each argument in an `ArgsView`.

5. `accumulate`
   ```cpp
   template <typename Func, typename Init, typename... Args>
   constexpr auto accumulate(Func&& func, Init init, ArgsViewT<Args...> args_view);
   ```
   Accumulates the arguments in an `ArgsView` using a function and an initial value.

## Operator Overloads

1. Equality Operators

   ```cpp
   template <typename... Args1, typename... Args2>
   constexpr auto operator==(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;

   template <typename... Args1, typename... Args2>
   constexpr auto operator!=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;
   ```

2. Comparison Operators

   ```cpp
   template <typename... Args1, typename... Args2>
   constexpr auto operator<(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;

   template <typename... Args1, typename... Args2>
   constexpr auto operator<=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;

   template <typename... Args1, typename... Args2>
   constexpr auto operator>(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;

   template <typename... Args1, typename... Args2>
   constexpr auto operator>=(ArgsView<Args1...> lhs, ArgsView<Args2...> rhs) -> bool;
   ```

## Utility Functions

1. `sum`

   ```cpp
   template <typename... Args>
   auto sum(Args&&... args) -> int;
   ```

   Sums the arguments.

2. `concat`

   ```cpp
   template <typename... Args>
   auto concat(Args&&... args) -> std::string;
   ```

   Concatenates the arguments into a string.

3. `print` (Available only in debug mode)
   ```cpp
   template <typename... Args>
   void print(Args&&... args);
   ```
   Prints the arguments to the standard output.
