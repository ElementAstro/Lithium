# RandomDataGenerator Class Documentation

## Overview

The `RandomDataGenerator` class is a powerful tool for generating random data for testing purposes, particularly useful in fuzz testing scenarios. It provides a wide range of methods to generate various types of data, from simple primitive types to complex structures like JSON, XML, and graphs.

## Class Definition

```cpp
class RandomDataGenerator {
public:
    explicit RandomDataGenerator(int seed = std::random_device{}());

    // ... (method declarations)

private:
    std::mt19937 generator_;
    std::uniform_int_distribution<> intDistribution_;
    std::uniform_real_distribution<> realDistribution_;
    std::uniform_int_distribution<> charDistribution_;

    // ... (private helper methods)
};
```

## Constructor

```cpp
explicit RandomDataGenerator(int seed = std::random_device{}());
```

Creates a new `RandomDataGenerator` object with an optional seed for the random number generator.

- **Parameters:**
  - `seed`: The seed for the random number generator (default: uses a random device).

## Public Methods

### Generate Primitive Types

1. **Generate Integers**

   ```cpp
   auto generateIntegers(int count, int min = 0, int max = DEFAULT_INT_MAX) -> std::vector<int>;
   ```

   Generates a vector of random integers.

2. **Generate Reals**

   ```cpp
   auto generateReals(int count, double min = 0.0, double max = 1.0) -> std::vector<double>;
   ```

   Generates a vector of random real numbers.

3. **Generate String**

   ```cpp
   auto generateString(int length, bool alphanumeric = false) -> std::string;
   ```

   Generates a random string.

4. **Generate Booleans**
   ```cpp
   auto generateBooleans(int count) -> std::vector<bool>;
   ```
   Generates a vector of random boolean values.

### Generate Complex Types

5. **Generate Exception**

   ```cpp
   auto generateException() -> std::string;
   ```

   Generates a random exception message.

6. **Generate DateTime**

   ```cpp
   auto generateDateTime(const std::chrono::system_clock::time_point& start,
                         const std::chrono::system_clock::time_point& end)
       -> std::chrono::system_clock::time_point;
   ```

   Generates a random date and time within a specified range.

7. **Generate Regex Match**

   ```cpp
   auto generateRegexMatch(const std::string& regexStr) -> std::string;
   ```

   Generates a string that matches a given regular expression.

8. **Generate File Path**

   ```cpp
   auto generateFilePath(const std::string& baseDir, int depth = 3) -> std::filesystem::path;
   ```

   Generates a random file path.

9. **Generate Random JSON**

   ```cpp
   auto generateRandomJSON(int depth = 2) -> std::string;
   ```

   Generates a random JSON string.

10. **Generate Random XML**
    ```cpp
    auto generateRandomXML(int depth = 2) -> std::string;
    ```
    Generates a random XML string.

### Network-related Generators

11. **Generate IPv4 Address**

    ```cpp
    auto generateIPv4Address() -> std::string;
    ```

    Generates a random IPv4 address.

12. **Generate MAC Address**

    ```cpp
    auto generateMACAddress() -> std::string;
    ```

    Generates a random MAC address.

13. **Generate URL**
    ```cpp
    auto generateURL() -> std::string;
    ```
    Generates a random URL.

### Statistical Distributions

14. **Generate Normal Distribution**

    ```cpp
    auto generateNormalDistribution(int count, double mean, double stddev) -> std::vector<double>;
    ```

    Generates a vector of random numbers following a normal distribution.

15. **Generate Exponential Distribution**
    ```cpp
    auto generateExponentialDistribution(int count, double lambda) -> std::vector<double>;
    ```
    Generates a vector of random numbers following an exponential distribution.

### Container Generators

16. **Generate Vector**

    ```cpp
    template <typename T>
    auto generateVector(int count, std::function<T()> generator) -> std::vector<T>;
    ```

    Generates a vector of random elements.

17. **Generate Map**

    ```cpp
    template <typename K, typename V>
    auto generateMap(int count, std::function<K()> keyGenerator,
                     std::function<V()> valueGenerator) -> std::map<K, V>;
    ```

    Generates a map of random key-value pairs.

18. **Generate Set**

    ```cpp
    template <typename T>
    auto generateSet(int count, std::function<T()> generator) -> std::set<T>;
    ```

    Generates a set of random elements.

19. **Generate Sorted Vector**

    ```cpp
    template <typename T>
    auto generateSortedVector(int count, std::function<T()> generator) -> std::vector<T>;
    ```

    Generates a sorted vector of random elements.

20. **Generate Unique Vector**
    ```cpp
    template <typename T>
    auto generateUniqueVector(int count, std::function<T()> generator) -> std::vector<T>;
    ```
    Generates a vector of unique random elements.

### Data Structure Generators

21. **Generate Tree**

    ```cpp
    auto generateTree(int depth, int maxChildren) -> TreeNode;
    ```

    Generates a random tree.

22. **Generate Graph**

    ```cpp
    auto generateGraph(int nodes, double edgeProbability) -> std::vector<std::vector<int>>;
    ```

    Generates a random graph represented as an adjacency matrix.

23. **Generate Key-Value Pairs**
    ```cpp
    auto generateKeyValuePairs(int count) -> std::vector<std::pair<std::string, std::string>>;
    ```
    Generates a vector of random key-value pairs.

### Utility Methods

24. **Serialize to JSON**

    ```cpp
    template <typename T>
    auto serializeToJSON(const T& data) -> std::string;
    ```

    Serializes data to a JSON string.

25. **Fuzz Test**
    ```cpp
    template <typename Func, typename... Args>
    void fuzzTest(Func testFunc, int iterations, std::function<Args()>... argGenerators);
    ```
    Performs a fuzz test on a given function.

## Usage Examples

### Example 1: Generating and Serializing Random Data

```cpp
#include "random_data_generator.hpp"
#include <iostream>

int main() {
    RandomDataGenerator generator;

    // Generate a vector of random integers
    auto integers = generator.generateIntegers(5, 1, 100);
    std::cout << "Random Integers: " << generator.serializeToJSON(integers) << std::endl;

    // Generate a random JSON string
    std::string json = generator.generateRandomJSON(3);
    std::cout << "Random JSON: " << json << std::endl;

    // Generate a random IPv4 address
    std::string ip = generator.generateIPv4Address();
    std::cout << "Random IP: " << ip << std::endl;

    return 0;
}
```

### Example 2: Fuzz Testing a Function

```cpp
#include "random_data_generator.hpp"
#include <iostream>

// Function to be fuzz tested
bool processData(const std::string& data, int value) {
    // Simulating some processing logic
    return data.length() > 5 && value > 10 && data.find('a') != std::string::npos;
}

int main() {
    RandomDataGenerator generator;

    // Perform fuzz testing
    generator.fuzzTest(
        processData,  // Function to test
        1000,         // Number of iterations
        [&]() { return generator.generateString(10); },  // String generator
        [&]() { return generator.generateIntegers(1, 0, 100)[0]; }  // Integer generator
    );

    return 0;
}
```

In this example, we're fuzz testing the `processData` function with randomly generated strings and integers. The `fuzzTest` method will call `processData` 1000 times with different combinations of random inputs.

### Example 3: Generating Complex Data Structures

```cpp
#include "random_data_generator.hpp"
#include <iostream>

int main() {
    RandomDataGenerator generator;

    // Generate a random tree
    auto tree = generator.generateTree(3, 3);
    std::cout << "Random Tree: " << generator.serializeToJSON(tree) << std::endl;

    // Generate a random graph
    auto graph = generator.generateGraph(5, 0.3);
    std::cout << "Random Graph: " << generator.serializeToJSON(graph) << std::endl;

    // Generate a map of string to vector of integers
    auto complexMap = generator.generateMap<std::string, std::vector<int>>(
        5,
        [&]() { return generator.generateString(5); },
        [&]() { return generator.generateIntegers(3, 1, 100); }
    );
    std::cout << "Complex Map: " << generator.serializeToJSON(complexMap) << std::endl;

    return 0;
}
```

This example demonstrates how to generate and serialize more complex data structures like trees, graphs, and nested containers.

### Example 4: Working with Custom Distributions

```cpp
#include "random_data_generator.hpp"
#include <iostream>

int main() {
    RandomDataGenerator generator;

    // Generate numbers following a normal distribution
    auto normalDist = generator.generateNormalDistribution(1000, 0.0, 1.0);
    std::cout << "Normal Distribution (first 10 values): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << normalDist[i] << " ";
    }
    std::cout << std::endl;

    // Generate numbers following an exponential distribution
    auto expDist = generator.generateExponentialDistribution(1000, 1.0);
    std::cout << "Exponential Distribution (first 10 values): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << expDist[i] << " ";
    }
    std::cout << std::endl;

    // Use a custom distribution (e.g., Poisson distribution)
    std::poisson_distribution<int> poissonDist(5.0);
    auto customDist = generator.generateCustomDistribution<int>(1000, poissonDist);
    std::cout << "Custom (Poisson) Distribution (first 10 values): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << customDist[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

This example shows how to work with different probability distributions, including custom ones.

## Best Practices

1. **Seed Management**:

   - Use a consistent seed when you need reproducible results.
   - Use a random seed (default behavior) for general testing to cover a wide range of scenarios.

2. **Error Handling**:

   - Wrap calls to generator methods in try-catch blocks to handle potential exceptions, especially when generating data that must conform to specific patterns (e.g., `generateRegexMatch`).

3. **Performance Considerations**:

   - For performance-critical applications, consider caching generated data or using more efficient generation methods for large datasets.

4. **Customization**:

   - Extend the `RandomDataGenerator` class or create custom generator functions for domain-specific data types.

5. **Fuzz Testing**:

   - Start with simple, well-understood functions when beginning fuzz testing.
   - Gradually increase complexity and adjust generator parameters based on initial results.

6. **Data Validation**:

   - Always validate the generated data, especially for complex types like JSON or XML, to ensure they meet your requirements.

7. **Resource Management**:
   - Be mindful of resource usage when generating large amounts of data or complex structures.

## Important Considerations

1. **Randomness Quality**:

   - The quality of randomness is suitable for testing but may not be cryptographically secure. Do not use for security-critical applications.

2. **Platform Dependence**:

   - Some methods (e.g., `generateFilePath`) may produce platform-specific results. Ensure your tests account for this.

3. **Performance Impact**:

   - Generating complex structures or large datasets can be computationally expensive. Profile your tests if performance is a concern.

4. **Memory Usage**:

   - Be aware of potential memory issues when generating very large data structures, especially with recursive methods like `generateTree`.

5. **Floating-Point Precision**:

   - When working with floating-point numbers (e.g., in `generateReals`), be mindful of precision limitations and potential rounding errors.

6. **Regular Expression Complexity**:

   - The `generateRegexMatch` method may struggle with very complex regular expressions. Start with simpler patterns and increase complexity gradually.

7. **Thread Safety**:
   - The current implementation is not guaranteed to be thread-safe. Use separate `RandomDataGenerator` instances for different threads if needed.

## Conclusion

The `RandomDataGenerator` class provides a versatile toolkit for generating random data in various formats and structures. It's particularly useful for fuzz testing, generating test data, and simulating complex scenarios in software testing. By following the best practices and considering the important points mentioned, you can effectively leverage this class to improve the quality and robustness of your software testing processes.
