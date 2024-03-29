# Custom Any Class

The `Any` class is a custom implementation in C++ that provides a way to store and access any type of value in a type-safe manner. It allows you to store values of different types in a single object and retrieve them when needed.

## Class Definition

The `Any` class has the following key features:

- **Constructors**:
  - `Any()`: Default constructor that creates an empty `Any` object.
  - `Any(T &&value)`: Constructor that initializes the `Any` object with the given value of type `T`.
  - `Any(const Any &other)`: Copy constructor to create a new `Any` object from another `Any` object.
  - `Any(Any &&other) noexcept`: Move constructor for efficient resource transfer.
- **Assignment Operators**:

  - `operator= (const Any &other)`: Assigns the value of another `Any` object to the current object.
  - `operator= (Any &&other) noexcept`: Move assignment operator for efficient resource transfer.
  - `operator= (T &&value)`: Assigns a new value of type `T` to the `Any` object.

- **Type Information**:

  - `type() const`: Returns the `std::type_info` object representing the stored type.

- **Utility Functions**:
  - `empty() const`: Checks if the `Any` object is empty.
  - `swap(Any &other)`: Swaps the contents of two `Any` objects.

## Function Templates

- **`any_cast(const Any &operand)`**:

  - Retrieves the stored value from an `Any` object as a const reference.
  - Throws `std::bad_cast` if the stored type does not match the requested type.

- **`any_cast(Any &&operand)`**:
  - Retrieves the stored value from an rvalue `Any` object.
  - Throws `std::bad_cast` if the stored type does not match the requested type.

## Usage Example

Here is an example demonstrating the usage of the `Any` class:

```cpp
Any a = 5;                                  // Storing an integer
std::cout << any_cast<int>(a) << std::endl; // Output: 5

a = std::string("Hello, Any!");                     // Storing a string
std::cout << any_cast<std::string>(a) << std::endl; // Output: Hello, Any!

// Trying to store an incompatible type
try {
  a = 3.14;
  std::cout << any_cast<int>(a) << std::endl; // Will throw std::bad_cast exception
} catch (const std::bad_cast &e) {
  std::cerr << "Bad cast caught: " << e.what() << std::endl;
}
```
