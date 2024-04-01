# Any

`Any` 类是 C++ 中的一个自定义实现，提供了一种以类型安全的方式存储和访问任何类型值的方法。它允许你在单个对象中存储不同类型的值，并在需要时检索这些值。

## 类定义

`Any` 类具有以下关键特性：

- **构造函数**:
  - `Any()`: 创建一个空的 `Any` 对象的默认构造函数。
  - `Any(T &&value)`: 使用给定类型为 `T` 的值初始化 `Any` 对象的构造函数。
  - `Any(const Any &other)`: 从另一个 `Any` 对象创建新的 `Any` 对象的复制构造函数。
  - `Any(Any &&other) noexcept`: 用于高效资源转移的移动构造函数。
- **赋值运算符**:

  - `operator= (const Any &other)`: 将另一个 `Any` 对象的值赋给当前对象。
  - `operator= (Any &&other) noexcept`: 用于高效资源转移的移动赋值运算符。
  - `operator= (T &&value)`: 将类型为 `T` 的新值赋给 `Any` 对象。

- **类型信息**:

  - `type() const`: 返回表示存储类型的 `std::type_info` 对象。

- **实用函数**:
  - `empty() const`: 检查 `Any` 对象是否为空。
  - `swap(Any &other)`: 交换两个 `Any` 对象的内容。

## 函数模板

- **`any_cast(const Any &operand)`**:

  - 作为常量引用从 `Any` 对象中检索存储的值。
  - 如果存储的类型与请求的类型不匹配，则抛出 `std::bad_cast` 异常。

- **`any_cast(Any &&operand)`**:
  - 从右值 `Any` 对象中检索存储的值。
  - 如果存储的类型与请求的类型不匹配，则抛出 `std::bad_cast` 异常。

## 使用示例

下面是一个演示 `Any` 类使用的示例：

```cpp
Any a = 5;                                  // 存储一个整数
std::cout << any_cast<int>(a) << std::endl; // 输出: 5

a = std::string("Hello, Any!");                     // 存储一个字符串
std::cout << any_cast<std::string>(a) << std::endl; // 输出: Hello, Any!

// 尝试存储不兼容的类型
try {
  a = 3.14;
  std::cout << any_cast<int>(a) << std::endl; // 将抛出 std::bad_cast 异常
} catch (const std::bad_cast &e) {
  std::cerr << "捕获到坏的转换: " << e.what() << std::endl;
}
```
