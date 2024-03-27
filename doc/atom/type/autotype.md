# C++ Template Class Examples

## Any Class

The `Any` class provides a way to store and retrieve values of any type. It uses template-based polymorphism to achieve this flexibility.

```cpp
Any a1 = 5;
int value = a1.cast<int>();
std::cout << "Value: " << value << std::endl;
```

Expected Output: Value: 5

- The `Any` class uses type erasure to store any type of value.
- The `cast` method is used to retrieve the stored value with the specified type.

## AutoType Class

The `AutoType` class is a template class that provides automatic type deduction for arithmetic operations. It enables automatic type conversion when performing arithmetic operations on objects of different types.

```cpp
AutoType<int> a = makeAutoType(5);
AutoType<double> b = makeAutoType(3.5);
auto result = a + b;
std::cout << "Result: " << result.m_value << std::endl;
```

Expected Output: Result: 8.5

- The `AutoType` class overloads various arithmetic operators to support automatic type deduction for arithmetic operations.
- The `makeAutoType` function template is used to create `AutoType` objects with automatic type deduction.

## TuplePrinter Class

The `TuplePrinter` class is a template class that provides functionality to print the elements of a tuple in reverse order.

```cpp
std::tuple<int, double, std::string> myTuple = std::make_tuple(10, 3.14, "Hello");
TuplePrinter<decltype(myTuple)>::print(myTuple);
```

Expected Output: Hello, 3.14, 10

- The `TuplePrinter` class uses template recursion to iterate through the elements of the tuple in reverse order.
- The `if constexpr` statement is used to enable compile-time branching based on the size of the tuple.

## Overall Example

```cpp
Any a1 = 5;
int value = a1.cast<int>();
std::cout << "Value: " << value << std::endl;

AutoType<int> a = makeAutoType(5);
AutoType<double> b = makeAutoType(3.5);
auto result = a + b;
std::cout << "Result: " << result.m_value << std::endl;

std::tuple<int, double, std::string> myTuple = std::make_tuple(10, 3.14, "Hello");
TuplePrinter<decltype(myTuple)>::print(myTuple);
```

Expected Output:
Value: 5
Result: 8.5
Hello, 3.14, 10
