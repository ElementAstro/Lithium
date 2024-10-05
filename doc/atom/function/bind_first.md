# bindFirst Function Documentation

## Overview

The `bindFirst` function is a utility in the `atom::meta` namespace that allows binding the first argument of a function to a specific object. This is particularly useful for creating function objects from member functions or for partial function application.

## Key Components

### Helper Functions

```cpp
template <typename T>
constexpr auto getPointer(T *ptr) noexcept -> T *;

template <typename T>
auto getPointer(const std::reference_wrapper<T> &ref) noexcept -> T *;

template <typename T>
constexpr auto getPointer(const T &ref) noexcept -> const T *;

template <typename T>
constexpr auto removeConstPointer(const T *ptr) noexcept -> T *;
```

These helper functions are used internally to handle different types of objects (pointers, references, and const pointers) uniformly.

### Concepts

```cpp
template <typename F, typename... Args>
concept invocable = std::is_invocable_v<F, Args...>;

template <typename F, typename... Args>
concept nothrow_invocable = std::is_nothrow_invocable_v<F, Args...>;
```

These concepts are used to constrain the `bindFirst` function templates, ensuring that the resulting function object is invocable with the given arguments.

## bindFirst Function Overloads

### For Free Functions

```cpp
template <typename O, typename Ret, typename P1, typename... Param>
constexpr auto bindFirst(Ret (*func)(P1, Param...), O &&object)
    requires invocable<Ret (*)(P1, Param...), O, Param...>;
```

Binds the first argument of a free function to an object.

### For Member Functions

```cpp
template <typename O, typename Ret, typename Class, typename... Param>
constexpr auto bindFirst(Ret (Class::*func)(Param...), O &&object)
    requires invocable<Ret (Class::*)(Param...), O, Param...>;

template <typename O, typename Ret, typename Class, typename... Param>
constexpr auto bindFirst(Ret (Class::*func)(Param...) const, O &&object)
    requires invocable<Ret (Class::*)(Param...) const, O, Param...>;
```

Binds the first argument (the object) of a member function to a specific instance.

### For std::function

```cpp
template <typename O, typename Ret, typename P1, typename... Param>
auto bindFirst(const std::function<Ret(P1, Param...)> &func, O &&object)
    requires invocable<std::function<Ret(P1, Param...)>, O, Param...>;
```

Binds the first argument of a `std::function` object to a specific value.

### For Function Objects

```cpp
template <typename F, typename O, typename Ret, typename Class, typename P1,
          typename... Param>
constexpr auto bindFirst(const F &funcObj, O &&object,
                         Ret (Class::*func)(P1, Param...) const)
    requires invocable<F, O, P1, Param...>;

template <typename F, typename O>
constexpr auto bindFirst(const F &func, O &&object)
    requires invocable<F, O>;

template <typename F, typename O>
constexpr auto bindFirst(F &&func, O &&object)
    requires std::invocable<F, O>;
```

These overloads handle various forms of function objects, including lambda expressions.

## Usage Examples

### Binding a Free Function

```cpp
int add(int a, int b) { return a + b; }

auto boundAdd = atom::meta::bindFirst(add, 5);
int result = boundAdd(3);  // result is 8
```

### Binding a Member Function

```cpp
class Calculator {
public:
    int multiply(int a, int b) const { return a * b; }
};

Calculator calc;
auto boundMultiply = atom::meta::bindFirst(&Calculator::multiply, calc);
int result = boundMultiply(4, 5);  // result is 20
```

### Binding a Lambda

```cpp
auto lambda = [](int x, int y) { return x - y; };
auto boundLambda = atom::meta::bindFirst(lambda, 10);
int result = boundLambda(3);  // result is 7
```

### Binding a std::function

```cpp
std::function<int(int, int)> divide = [](int a, int b) { return a / b; };
auto boundDivide = atom::meta::bindFirst(divide, 20);
int result = boundDivide(4);  // result is 5
```

## Notes

- The `bindFirst` function uses perfect forwarding to handle both lvalue and rvalue references efficiently.
- The use of concepts ensures that the function will only compile if the binding operation is valid.
- The implementation handles const-correctness, allowing binding to both const and non-const member functions.
- The resulting function object captures the bound object by value, which may have implications for lifetime and performance.
- When binding to member functions, the implementation uses `removeConstPointer` to handle const objects correctly.
