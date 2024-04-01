# C++ Function Decorator

## Introduction

This is a C++ library for function decoration, providing a way to add additional behavior to functions without modifying their code directly. It allows adding hooks, condition checks, and looping to existing functions.

## Usage

### Basic Decoration

To use the function decorator, first include the necessary headers and define your function. Then, create a decorator using the `make_decorator` function and add hooks or other behaviors using the `with_hooks` method.

```cpp
#include "function_decorator.h"

void myFunction(int arg) {
    std::cout << "Executing function with arg " << arg << std::endl;
}

int main() {
    auto decoratedFunc = make_decorator(myFunction).with_hooks(
        []() { std::cout << "Before function execution" << std::endl; },
        [](int result) {
            std::cout << "After function execution with result " << result
                      << std::endl;
        },
        [](long long timeTaken) {
            std::cout << "Function execution took " << timeTaken << " microseconds"
                      << std::endl;
        });

    decoratedFunc(42);

    return 0;
}
```

### Loop Decoration

You can also create a loop decorator using the `make_loop_decorator` function to execute the function multiple times in a loop.

```cpp
auto loopDecoratedFunc = make_loop_decorator(myFunction);
loopDecoratedFunc(3, 42);  // Execute myFunction 3 times with arg 42
```

### Condition Check Decoration

Condition check decorators can be created to conditionally execute the function based on a condition.

```cpp
auto conditionDecoratedFunc = make_condition_check_decorator(myFunction);
conditionDecoratedFunc([]() { return true; }, 42);  // Execute myFunction if condition is true
```

### Chaining Decorators

Multiple decorators can be chained together using `DecorateStepper` class to create a sequence of decorators that are applied to the function.

```cpp
DecorateStepper<int, int> stepper(myFunction);
stepper.addDecorator<LoopDecorator>(3); // Loop 3 times
stepper.addDecorator<ConditionCheckDecorator>([]() { return true; }); // Conditionally execute
stepper.execute(42);  // Execute the decorated function
```

## Conclusion

The function decorator library provides a flexible way to add functionality to existing functions, allowing for easy extension and modification of behavior without directly altering the original function code.
