# 分数类

Fraction 类表示一个分数，并提供基本的分数运算和功能。

## 成员函数

### 构造函数

#### `Fraction(int num_value, int den_value)`

使用给定的分子和分母构造一个分数对象。

#### `Fraction(int num_value)`

使用给定的整数作为分子，将分母默认设置为 1 来构造一个分数对象。

#### `Fraction(const char *str)`

从给定的字符串构造一个分数对象。

#### `Fraction()`

默认构造函数，构造一个值为 0 的分数对象。

### 成员方法

#### `int getNumerator() const`

获取分数的分子。

#### `int getDenominator() const`

获取分数的分母。

#### `void alterValue(int num_value, int den_value)`

将分数的值更改为指定的分子和分母。

#### `void alterValue(const Fraction &f)`

将分数的值更改为另一个分数对象的值。

#### `Fraction inverse()`

返回分数的倒数。

### 运算符重载

#### `+`, `-`, `*`, `/`

实现分数的加法、减法、乘法和除法运算。

#### `==`, `!=`, `>`, `>=`, `<`, `<=`

实现分数的比较运算符。

#### `<<`, `>>`

为分数对象实现输入和输出流操作。

#### 取反运算符

将分数取反为其相反值。

### 友元函数

#### `operator>>`

从输入流中读取分数对象的值。

#### `operator<<`

将分数对象的值写入输出流。

## 示例

```cpp
Fraction f1(1, 2);
Fraction f2(3, 4);
Fraction result = f1 + f2; // result = 5/4
std::cout << result; // 输出: 5/4
```

```cpp
Fraction f1(2, 3);
Fraction f2(1, 6);
bool isEqual = (f1 == f2); // isEqual = false
```

```cpp
Fraction f1(3, 5);
Fraction f2 = -f1; // f2 = -3/5
```
