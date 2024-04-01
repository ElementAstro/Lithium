# Callable Traits Library

The `Callable` Traits library provides a set of utilities to handle callable objects, member functions, and free functions in C++. It offers mechanisms to call these functions and to obtain their signatures and return types.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
  - [Constructor](#constructor)
  - [Const_Caller](#const_caller)
  - [Fun_Caller](#fun_caller)
  - [Caller](#caller)
  - [Arity](#arity)
  - [Function_Signature](#function_signature)
  - [Callable_Traits](#callable_traits)
- [Examples](#examples)

## Features

- Create objects of a given class using `Constructor`.
- Call const member functions using `Const_Caller`.
- Call free functions using `Fun_Caller`.
- Call member functions using `Caller`.
- Determine the arity of a function using `Arity`.
- Obtain the signature and return type of a function using `Function_Signature`.
- Obtain the signature and return type of a callable object using `Callable_Traits`.

## Installation

The library is provided as a header-only template. Simply include the header file in your project.

```cpp
#include "callable.hpp"
```

## Usage

### Constructor

The `Constructor` struct creates objects of a given class.

```cpp
template <typename Class, typename... Param>
struct Constructor {
    template <typename... Inner>
    std::shared_ptr<Class> operator()(Inner &&...inner) const;
};
```

**Example:**

```cpp
struct MyClass {
    MyClass(int a, int b) : val(a + b) {}
    int val;
};

Constructor<MyClass, int, int> constructor;
auto obj = constructor(3, 4);
```

### Const_Caller

The `Const_Caller` struct calls const member functions of a class.

```cpp
template <typename Ret, typename Class, typename... Param>
struct Const_Caller {
    Ret operator()(const Class &o, Inner &&...inner) const;
};
```

**Example:**

```cpp
struct MyClass {
    int add(int a, int b) const { return a + b; }
};

Const_Caller<int, MyClass, int, int> caller(&MyClass::add);
MyClass obj;
int result = caller(obj, 3, 4);
```

### Fun_Caller

The `Fun_Caller` struct calls free functions.

```cpp
template <typename Ret, typename... Param>
struct Fun_Caller {
    Ret operator()(Inner &&...inner) const;
};
```

**Example:**

```cpp
int add(int a, int b) { return a + b; }

Fun_Caller<int, int, int> caller(&add);
int result = caller(3, 4);
```

### Caller

The `Caller` struct calls member functions of a class.

```cpp
template <typename Ret, typename Class, typename... Param>
struct Caller {
    Ret operator()(Class &o, Inner &&...inner) const;
};
```

**Example:**

```cpp
struct MyClass {
    int add(int a, int b) { return a + b; }
};

Caller<int, MyClass, int, int> caller(&MyClass::add);
MyClass obj;
int result = caller(obj, 3, 4);
```

### Arity

The `Arity` struct determines the arity (number of parameters) of a function.

```cpp
template <typename T>
struct Arity {};

template <typename Ret, typename... Params>
struct Arity<Ret(Params...)> {
    static const size_t arity;
};
```

**Example:**

```cpp
Arity<int(int, int)>::arity;  // Returns 2
```

### Function_Signature

The `Function_Signature` struct obtains the signature and return type of a function.

```cpp
template <typename T>
struct Function_Signature {};

template <typename Ret, typename... Params>
struct Function_Signature<Ret(Params...)> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);
};
```

**Example:**

```cpp
Function_Signature<int(int, int)>::Signature;  // Returns int (*)(int, int)
```

### Callable_Traits

The `Callable_Traits` struct obtains the signature and return type of a callable object.

```cpp
template <typename T>
struct Callable_Traits {
    using Signature;
    using Return_Type;
};
```

**Example:**

```cpp
struct MyCallable {
    int operator()(int a, int b) const { return a * b; }
};

Callable_Traits<MyCallable>::Signature;  // Returns int (*)(int, int)
Callable_Traits<MyCallable>::Return_Type;  // Returns int
```

## Examples

Here are some comprehensive examples demonstrating the usage of the library:

```cpp
#include "callable.hpp"
#include <iostream>

// Define a simple function
int add(int a, int b) { return a + b; }

// Define a callable object
struct MyCallable {
    int operator()(int a, int b) const { return a * b; }
};

int main() {
    using namespace std;
    using namespace chaiscript::dispatch::detail;

    // Call a free function
    Fun_Caller<int, int, int> funCaller(&add);
    cout << "Addition result: " << funCaller(3, 4) << endl;

    // Call a member function
    struct MyClass {
        int add(int a, int b) { return a + b; }
    };
    Caller<int, MyClass, int, int> myCaller(&MyClass::add);
    MyClass obj;
    cout << "MyClass addition result: " << myCaller(obj, 3, 4) << endl;

    // Call a const member function
    Const_Caller<int, MyCallable, int, int> constCaller(&MyCallable::operator());
    MyCallable myCallable;
    cout << "Multiplication result: " << constCaller(myCallable, 5, 6) << endl;

    // Obtain the signature and return type of a callable object
    cout << "Signature of MyCallable: " << typeid(Callable_Traits<MyCallable>::Signature).name() << endl;
    cout << "Return type of MyCallable: " << typeid(Callable_Traits<MyCallable>::Return_Type).name() << endl;

    return 0;
}
```
