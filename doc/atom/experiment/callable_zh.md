# Callable Traits

`Callable` Traits 提供了一组实用工具，用于处理可调用对象、成员函数和自由函数在 C++ 中的使用。它提供了调用这些函数的机制以及获取它们的签名和返回类型的功能。

## 目录

- [特点](#特点)
- [安装](#安装)
- [用法](#用法)
  - [Constructor](#constructor)
  - [Const_Caller](#const_caller)
  - [Fun_Caller](#fun_caller)
  - [Caller](#caller)
  - [Arity](#arity)
  - [Function_Signature](#function_signature)
  - [Callable_Traits](#callable_traits)
- [示例](#示例)

## 特点

- 使用 `Constructor` 创建给定类的对象。
- 使用 `Const_Caller` 调用类的 const 成员函数。
- 使用 `Fun_Caller` 调用自由函数。
- 使用 `Caller` 调用类的成员函数。
- 使用 `Arity` 确定函数的参数个数。
- 使用 `Function_Signature` 获取函数的签名和返回类型。
- 使用 `Callable_Traits` 获取可调用对象的签名和返回类型。

## 安装

该库提供为头文件模板。只需在项目中包含头文件即可。

```cpp
#include "callable.hpp"
```

## 用法

### Constructor

`Constructor` 结构体创建给定类的对象。

```cpp
template <typename Class, typename... Param>
struct Constructor {
    template <typename... Inner>
    std::shared_ptr<Class> operator()(Inner &&...inner) const;
};
```

**示例：**

```cpp
struct MyClass {
    MyClass(int a, int b) : val(a + b) {}
    int val;
};

Constructor<MyClass, int, int> constructor;
auto obj = constructor(3, 4);
```

### Const_Caller

`Const_Caller` 结构体调用类的 const 成员函数。

```cpp
template <typename Ret, typename Class, typename... Param>
struct Const_Caller {
    Ret operator()(const Class &o, Inner &&...inner) const;
};
```

**示例：**

```cpp
struct MyClass {
    int add(int a, int b) const { return a + b; }
};

Const_Caller<int, MyClass, int, int> caller(&MyClass::add);
MyClass obj;
int result = caller(obj, 3, 4);
```

### Fun_Caller

`Fun_Caller` 结构体调用自由函数。

```cpp
template <typename Ret, typename... Param>
struct Fun_Caller {
    Ret operator()(Inner &&...inner) const;
};
```

**示例：**

```cpp
int add(int a, int b) { return a + b; }

Fun_Caller<int, int, int> caller(&add);
int result = caller(3, 4);
```

### Caller

`Caller` 结构体调用类的成员函数。

```cpp
template <typename Ret, typename Class, typename... Param>
struct Caller {
    Ret operator()(Class &o, Inner &&...inner) const;
};
```

**示例：**

```cpp
struct MyClass {
    int add(int a, int b) { return a + b; }
};

Caller<int, MyClass, int, int> caller(&MyClass::add);
MyClass obj;
int result = caller(obj, 3, 4);
```

### Arity

`Arity` 结构体确定函数的参数个数。

```cpp
template <typename T>
struct Arity {};

template <typename Ret, typename... Params>
struct Arity<Ret(Params...)> {
    static const size_t arity;
};
```

**示例：**

```cpp
Arity<int(int, int)>::arity;  // 返回 2
```

### Function_Signature

`Function_Signature` 结构体获取函数的签名和返回类型。

```cpp
template <typename T>
struct Function_Signature {};

template <typename Ret, typename... Params>
struct Function_Signature<Ret(Params...)> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);
};
```

**示例：**

```cpp
Function_Signature<int(int, int)>::Signature;  // 返回 int (*)(int, int)
```

### Callable_Traits

`Callable_Traits` 结构体获取可调用对象的签名和返回类型。

```cpp
template <typename T>
struct Callable_Traits {
    using Signature;
    using Return_Type;
};
```

**示例：**

```cpp
struct MyCallable {
    int operator()(int a, int b) const { return a * b; }
};

Callable_Traits<MyCallable>::Signature;  // 返回 int (*)(int, int)
Callable_Traits<MyCallable>::Return_Type;  // 返回 int
```

## 示例

这里有一些全面的示例，演示了库的使用：

```cpp
#include "callable.hpp"
#include <iostream>

// 定义一个简单的函数
int add(int a, int b) { return a + b; }

// 定义一个可调用对象
struct MyCallable {
    int operator()(int a, int b) const { return a * b; }
};

int main() {
    using namespace std;
    using namespace chaiscript::dispatch::detail;

    // 调用自由函数
    Fun_Caller<int, int, int> funCaller(&add);
    cout << "加法结果：" << funCaller(3, 4) << endl;

    // 调用类的成员函数
    struct MyClass {
        int add(int a, int b) { return a + b; }
    };
    Caller<int, MyClass, int, int> myCaller(&MyClass::add);
    MyClass obj;
    cout << "MyClass 加法结果：" << myCaller(obj, 3, 4) << endl;

    // 调用类的 const 成员函数
    Const_Caller<int, MyCallable, int, int> constCaller(&MyCallable::operator());
    MyCallable myCallable;
    cout << "乘法结果：" << constCaller(myCallable, 5, 6) << endl;

    // 获取可调用对象的签名和返回类型
    cout << "MyCallable 的签名：" << typeid(Callable_Traits<MyCallable>::Signature).name() << endl;
    cout << "MyCallable 的返回类型：" << typeid(Callable_Traits<MyCallable>::Return_Type).name() << endl;

    return 0;
}
```
