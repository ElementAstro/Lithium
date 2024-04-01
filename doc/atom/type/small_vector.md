# SmallVector Template Class

The `SmallVector` class is a template class that provides a dynamic array implementation with small buffer optimization. It uses a fixed-size inline storage for a small number of elements and switches to dynamic allocation when the number of elements exceeds the capacity of the inline storage.

## Constructor

```cpp
SmallVector<int, 3> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);
```

Expected Output: N/A

- The constructor initializes the vector with inline storage and sets the initial size and capacity.

## Copy Constructor

```cpp
SmallVector<int, 3> vec1;
vec1.push_back(1);
vec1.push_back(2);
SmallVector<int, 3> vec2 = vec1;
```

Expected Output: N/A

- Copies elements from another `SmallVector` instance with proper handling for inline and dynamic data.

## Move Constructor

```cpp
SmallVector<int, 3> vec1;
vec1.push_back(1);
vec1.push_back(2);
SmallVector<int, 3> vec2 = std::move(vec1);
```

Expected Output: N/A

- Moves elements from another `SmallVector` instance with efficient handling of data ownership.

## Destructor

```cpp
{
    SmallVector<int, 3> vec;
    vec.push_back(1);
    // Destructor called automatically when vec goes out of scope
}
```

Expected Output: N/A

- Properly deallocates memory if dynamic allocation was used.

## Assignment Operator

```cpp
SmallVector<int, 3> vec1;
vec1.push_back(1);
SmallVector<int, 3> vec2;
vec2 = vec1;
```

Expected Output: N/A

- Handles assignment of one `SmallVector` to another.

## Element Access and Modification

```cpp
SmallVector<int, 3> vec;
vec.push_back(1);
vec.push_back(2);
int value = vec[1];
std::cout << "Value at index 1: " << value << std::endl;
```

Expected Output: Value at index 1: 2

- Provides access to elements using the subscript operator.

## Additional Operations

```cpp
SmallVector<int, 3> vec;
vec.push_back(1);
vec.push_back(2);
vec.pop_back();
std::cout << "Size: " << vec.size() << std::endl;
```

Expected Output: Size: 1

- Supports operations like adding, removing elements, clearing the vector, and checking size and capacity.

## Overall Example

```cpp
SmallVector<int, 3> vec1;
vec1.push_back(1);
vec1.push_back(2);

SmallVector<int, 3> vec2 = vec1;
vec2.push_back(3);

for (size_t i = 0; i < vec2.size(); ++i) {
    std::cout << vec2[i] << " ";
}
std::cout << std::endl;
```

Expected Output: 1 2 3
