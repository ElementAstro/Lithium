# Template Traits and Utilities Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Basic Template Traits](#basic-template-traits)
3. [Template Specialization Checks](#template-specialization-checks)
4. [Template Parameter Extraction](#template-parameter-extraction)
5. [Function Traits](#function-traits)
6. [Constraint Level Checks](#constraint-level-checks)
7. [Base Template Checks](#base-template-checks)
8. [Usage Examples](#usage-examples)

## Introduction

The `template_traits.hpp` file provides a comprehensive set of template metaprogramming utilities for C++20. These utilities help in analyzing and manipulating template types, checking for specializations, extracting template parameters, and more.

## Basic Template Traits

### `is_template`

Checks if a type is a template instantiation.

```cpp
template <typename T>
inline constexpr bool is_template_v = is_template<T>::value;
```

### `template_traits`

Extracts template parameters and full name of a template instantiation.

```cpp
template <typename T>
struct template_traits;
```

### `args_type_of`

Alias for extracting the tuple of template arguments.

```cpp
template <typename T>
using args_type_of = typename template_traits<T>::args_type;
```

### `template_arity_v`

Constant for the number of template arguments.

```cpp
template <typename T>
inline constexpr std::size_t template_arity_v = std::tuple_size_v<args_type_of<T>>;
```

## Template Specialization Checks

### `is_specialization_of`

Checks if a type is a specialization of a given template.

```cpp
template <template <typename...> typename Template, typename T>
inline constexpr bool is_specialization_of_v = is_specialization_of<Template, T>::value;
```

### `is_partial_specialization_of`

Checks if a type is a partial specialization of a given template.

```cpp
template <typename T, template <typename, typename...> typename Template>
inline constexpr bool is_partial_specialization_of_v = is_partial_specialization_of<T, Template>::value;
```

## Template Parameter Extraction

### `template_arg_t`

Extracts the N-th template parameter type.

```cpp
template <std::size_t N, typename T>
using template_arg_t = std::tuple_element_t<N, args_type_of<T>>;
```

### `count_occurrences_v`

Counts the number of occurrences of a type in a parameter pack.

```cpp
template <typename T, typename... Args>
constexpr std::size_t count_occurrences_v = (0 + ... + std::is_same_v<T, Args>);
```

### `find_first_index_v`

Finds the index of the first occurrence of a type in a parameter pack.

```cpp
template <typename T, typename... Args>
constexpr std::size_t find_first_index_v = /* implementation */;
```

## Function Traits

### `extract_function_traits`

Extracts function return type and parameter types.

```cpp
template <typename T>
struct extract_function_traits;
```

### `extract_function_return_type_t`

Alias for extracting the return type of a function.

```cpp
template <typename T>
using extract_function_return_type_t = typename extract_function_traits<T>::return_type;
```

### `extract_function_parameters_t`

Alias for extracting the parameter types of a function.

```cpp
template <typename T>
using extract_function_parameters_t = typename extract_function_traits<T>::parameter_types;
```

## Constraint Level Checks

### `constraint_level`

Enum class for different levels of constraints.

```cpp
enum class constraint_level { none, nontrivial, nothrow, trivial };
```

### Constraint Check Functions

```cpp
template <typename T>
consteval bool has_copyability(constraint_level level);

template <typename T>
consteval bool has_relocatability(constraint_level level);

template <typename T>
consteval bool has_destructibility(constraint_level level);
```

## Base Template Checks

### `is_base_of_template_v`

Checks if a type is derived from a template base class.

```cpp
template <template <typename...> class Base, typename Derived>
inline constexpr bool is_base_of_template_v = is_base_of_template_impl<Base, Derived>::value;
```

### `is_base_of_any_template_v`

Checks if a type is derived from any of the given template base classes.

```cpp
template <typename Derived, template <typename...> class... Bases>
inline constexpr bool is_base_of_any_template_v = is_base_of_any_template<Derived, Bases...>::value;
```

## Usage Examples

Here are some examples demonstrating how to use these template traits and utilities:

```cpp
#include "template_traits.hpp"
#include <iostream>
#include <vector>
#include <list>

template <typename T>
struct MyTemplate {};

class MyClass : public MyTemplate<int> {};

int main() {
    // Basic template checks
    std::cout << "Is vector<int> a template? " << atom::meta::is_template_v<std::vector<int>> << std::endl;
    std::cout << "Is int a template? " << atom::meta::is_template_v<int> << std::endl;

    // Template specialization checks
    std::cout << "Is vector<int> a specialization of vector? "
              << atom::meta::is_specialization_of_v<std::vector, std::vector<int>> << std::endl;

    // Template parameter extraction
    using VectorInt = std::vector<int>;
    std::cout << "First template argument of vector<int>: "
              << typeid(atom::meta::template_arg_t<0, VectorInt>).name() << std::endl;

    // Function traits
    using FuncType = int(double, char);
    std::cout << "Return type of FuncType: "
              << typeid(atom::meta::extract_function_return_type_t<FuncType>).name() << std::endl;

    // Constraint level checks
    std::cout << "Is int trivially copyable? "
              << atom::meta::has_copyability<int>(atom::meta::constraint_level::trivial) << std::endl;

    // Base template checks
    std::cout << "Is MyClass derived from MyTemplate? "
              << atom::meta::is_base_of_template_v<MyTemplate, MyClass> << std::endl;

    return 0;
}
```

This example demonstrates various use cases for the template traits and utilities provided in the `template_traits.hpp` file. It shows how to check for template instantiations, specializations, extract template parameters, analyze function types, and check for base template relationships.
