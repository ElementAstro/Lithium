# Atom Test Framework

The Atom Test Framework is a comprehensive C++ testing library designed for ease of use, flexibility, and performance. It provides a rich set of features for writing and running unit tests, including support for asynchronous testing, parallel test execution, and various assertion types.

## Table of Contents

1. [Key Features](#key-features)
2. [Core Components](#core-components)
3. [Test Registration and Execution](#test-registration-and-execution)
4. [Assertions](#assertions)
5. [Advanced Features](#advanced-features)
6. [Usage Examples](#usage-examples)

## Key Features

- Simple test case registration
- Support for test suites
- Asynchronous test execution
- Parallel test execution
- Customizable test hooks (before/after each, before/after all)
- Comprehensive set of assertions
- Test result reporting (console output and exportable formats)
- Test filtering and dependency management
- Retry mechanism for flaky tests

## Core Components

### TestCase

The `TestCase` struct represents an individual test case:

```cpp
struct TestCase {
    std::string name;
    std::function<void()> func;
    bool skip = false;
    bool async = false;
    double timeLimit = 0.0;
    std::vector<std::string> dependencies;
};
```

### TestResult

The `TestResult` struct stores the outcome of a test case:

```cpp
struct TestResult {
    std::string name;
    bool passed;
    bool skipped;
    std::string message;
    double duration;
    bool timedOut;
};
```

### TestSuite

The `TestSuite` struct groups related test cases:

```cpp
struct TestSuite {
    std::string name;
    std::vector<TestCase> testCases;
};
```

### TestStats

The `TestStats` struct keeps track of overall test statistics:

```cpp
struct TestStats {
    int totalTests = 0;
    int totalAsserts = 0;
    int passedAsserts = 0;
    int failedAsserts = 0;
    int skippedTests = 0;
    std::vector<TestResult> results;
};
```

## Test Registration and Execution

### Registering Tests

Tests can be registered using the `registerTest` function or the `_test` user-defined literal:

```cpp
registerTest("Test Name", []() { /* Test code */ });

"Test Name"_test([]() { /* Test code */ }, async, timeLimit, skip, dependencies);
```

### Running Tests

Tests can be run using the `runTests` function, which accepts command-line arguments:

```cpp
int main(int argc, char* argv[]) {
    atom::test::runTests(argc, argv);
    return 0;
}
```

Command-line options include:

- `--retry <count>`: Number of retries for failed tests
- `--parallel <threads>`: Enable parallel execution with specified number of threads
- `--export <format> <filename>`: Export test results in specified format (json, xml, html)

## Assertions

The framework provides a wide range of assertion macros:

- `expect(expr)`: General assertion
- `expect_eq(lhs, rhs)`: Equality assertion
- `expect_ne(lhs, rhs)`: Inequality assertion
- `expect_gt(lhs, rhs)`: Greater than assertion
- `expect_lt(lhs, rhs)`: Less than assertion
- `expect_ge(lhs, rhs)`: Greater than or equal assertion
- `expect_le(lhs, rhs)`: Less than or equal assertion
- `expect_approx(lhs, rhs, eps)`: Approximate equality assertion
- `expect_contains(str, substr)`: String contains assertion
- `expect_set_eq(lhs, rhs)`: Set equality assertion

## Advanced Features

### Asynchronous Testing

Tests can be marked as asynchronous and given a time limit:

```cpp
"Async Test"_test([]() { /* Async test code */ }, true, 1000.0);
```

### Parallel Execution

Tests can be run in parallel by specifying the `--parallel` option:

```bash
./test_program --parallel 4
```

### Test Hooks

Custom hooks can be set for test execution:

```cpp
getHooks().beforeEach = []() { /* Setup before each test */ };
getHooks().afterEach = []() { /* Cleanup after each test */ };
getHooks().beforeAll = []() { /* Setup before all tests */ };
getHooks().afterAll = []() { /* Cleanup after all tests */ };
```

### Test Filtering

Tests can be filtered using regular expressions:

```cpp
auto filteredTests = filterTests(std::regex("TestPattern"));
```

### Test Dependencies

Tests can specify dependencies on other tests:

```cpp
"Dependent Test"_test([]() { /* Test code */ }, false, 0.0, false, {"Prerequisite Test"});
```

## Usage Examples

Here's a simple example of how to use the Atom Test Framework:

```cpp
#include "atom_test.hpp"

using namespace atom::test;

int main(int argc, char* argv[]) {
    "Addition Test"_test([]() {
        expect_eq(2 + 2, 4);
    });

    "Async Test"_test([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        expect(true);
    }, true, 200.0);

    runTests(argc, argv);
    return 0;
}
```

This example demonstrates basic test registration, an asynchronous test, and running the tests with command-line argument support.

The Atom Test Framework provides a powerful and flexible solution for C++ unit testing, suitable for both small projects and large-scale applications.
