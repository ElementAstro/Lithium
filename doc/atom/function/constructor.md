# Constructor and Function Binding Utilities Documentation

## Overview

This document covers a set of utility functions in the `atom::meta` namespace designed to facilitate function binding, constructor creation, and object instantiation. These utilities are particularly useful for metaprogramming, reflection, and creating flexible object factories.

## Function Binding Utilities

### bindMemberFunction

```cpp
template <typename MemberFunc, typename ClassType>
auto bindMemberFunction(MemberFunc ClassType::*member_func);
```

Binds a member function to an object.

- **Parameters:**
  - `member_func`: Pointer to the member function.
- **Returns:** A lambda that binds the member function to an object.
- **Usage:**

  ```cpp
  auto bound = bindMemberFunction(&MyClass::myMethod);
  MyClass obj;
  bound(obj, arg1, arg2);
  ```

### bindStaticFunction

```cpp
template <typename Func>
auto bindStaticFunction(Func func);
```

Binds a static function.

- **Parameters:**
  - `func`: The static function.
- **Returns:** The static function itself.
- **Usage:**

  ```cpp
  auto bound = bindStaticFunction(myStaticFunction);
  bound(arg1, arg2);
  ```

### bindMemberVariable

```cpp
template <typename MemberType, typename ClassType>
auto bindMemberVariable(MemberType ClassType::*member_var);
```

Binds a member variable to an object.

- **Parameters:**
  - `member_var`: Pointer to the member variable.
- **Returns:** A lambda that binds the member variable to an object.
- **Usage:**

  ```cpp
  auto bound = bindMemberVariable(&MyClass::myVariable);
  MyClass obj;
  auto& var = bound(obj);
  ```

## Constructor Utilities

### buildSharedConstructor

```cpp
template <typename Class, typename... Params>
auto buildSharedConstructor(Class (* /*unused*/)(Params...));
```

Builds a shared constructor for a class.

- **Returns:** A lambda that constructs a shared pointer to the class.
- **Usage:**

  ```cpp
  auto ctor = buildSharedConstructor(static_cast<MyClass*(*)(int, string)>(nullptr));
  auto instance = ctor(42, "Hello");
  ```

### buildCopyConstructor

```cpp
template <typename Class, typename... Params>
auto buildCopyConstructor(Class (* /*unused*/)(Params...));
```

Builds a copy constructor for a class.

- **Returns:** A lambda that constructs an instance of the class.
- **Usage:**

  ```cpp
  auto ctor = buildCopyConstructor(static_cast<MyClass*(*)(const MyClass&)>(nullptr));
  MyClass original;
  auto copy = ctor(original);
  ```

### buildPlainConstructor

```cpp
template <typename Class, typename... Params>
auto buildPlainConstructor(Class (* /*unused*/)(Params...));
```

Builds a plain constructor for a class.

- **Returns:** A lambda that constructs an instance of the class.
- **Usage:**

  ```cpp
  auto ctor = buildPlainConstructor(static_cast<MyClass*(*)(int, string)>(nullptr));
  auto instance = ctor(42, "Hello");
  ```

### buildConstructor

```cpp
template <typename Class, typename... Args>
auto buildConstructor();
```

Builds a constructor for a class with specified arguments.

- **Returns:** A lambda that constructs a shared pointer to the class.
- **Usage:**

  ```cpp
  auto ctor = buildConstructor<MyClass, int, string>();
  auto instance = ctor(42, "Hello");
  ```

### buildDefaultConstructor

```cpp
template <typename Class>
auto buildDefaultConstructor();
```

Builds a default constructor for a class.

- **Returns:** A lambda that constructs an instance of the class.
- **Usage:**

  ```cpp
  auto ctor = buildDefaultConstructor<MyClass>();
  auto instance = ctor();
  ```

### buildMoveConstructor

```cpp
template <typename Class>
auto buildMoveConstructor();
```

Constructs an instance of a class using a move constructor.

- **Returns:** A lambda that constructs an instance of the class using a move constructor.
- **Usage:**

  ```cpp
  auto ctor = buildMoveConstructor<MyClass>();
  MyClass original;
  auto moved = ctor(std::move(original));
  ```

### buildInitializerListConstructor

```cpp
template <typename Class, typename T>
auto buildInitializerListConstructor();
```

Constructs an instance of a class using an initializer list.

- **Returns:** A lambda that constructs an instance of the class using an initializer list.
- **Usage:**

  ```cpp
  auto ctor = buildInitializerListConstructor<MyClass, int>();
  auto instance = ctor({1, 2, 3, 4, 5});
  ```

## High-Level Constructor Functions

### constructor

```cpp
template <typename T>
auto constructor();

template <typename Class, typename... Args>
auto constructor();
```

Constructs an instance of a class based on its traits or with specified arguments.

- **Returns:** A lambda that constructs an instance of the class.
- **Usage:**

  ```cpp
  auto ctor1 = constructor<MyClass(int, string)>();
  auto instance1 = ctor1(42, "Hello");

  auto ctor2 = constructor<MyClass, int, string>();
  auto instance2 = ctor2(42, "Hello");
  ```

### defaultConstructor

```cpp
template <typename Class>
auto defaultConstructor();
```

Constructs an instance of a class using the default constructor.

- **Returns:** A lambda that constructs an instance of the class.
- **Throws:** Exception if the class is not default constructible.
- **Usage:**

  ```cpp
  auto ctor = defaultConstructor<MyClass>();
  auto instance = ctor();
  ```

## Notes

- These utilities make extensive use of C++17 and C++20 features, including perfect forwarding, constexpr if, and concepts.
- The `constructor` function uses SFINAE to select between shared and copy constructors based on the class's traits.
- Many of these functions return lambdas, allowing for flexible and efficient use in various contexts.
- The utilities handle const-correctness and move semantics appropriately.
- Error handling is implemented for cases where a requested operation is not possible (e.g., default construction for a class without a default constructor).
