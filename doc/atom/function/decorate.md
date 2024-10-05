# Decorator System Documentation

## Overview

This document covers the decorator system implemented in the `atom::meta` namespace. The system provides a flexible way to add behavior to functions without modifying their code, similar to Python's decorators.

## Key Components

### Switchable

A class that allows switching between different implementations of a function.

```cpp
template <typename Func>
class Switchable {
    // ... implementation ...
};
```

#### Usage:

```cpp
auto switchable = Switchable([](int a, int b) { return a + b; });
switchable.switchTo([](int a, int b) { return a * b; });
int result = switchable(3, 4);  // result is 12
```

### decorator

A class template that allows adding pre-execution, post-execution, and callback hooks to a function.

```cpp
template <typename R, typename... Args>
struct decorator<R(Args...)> {
    // ... implementation ...
};
```

#### Usage:

```cpp
auto decoratedFunc = makeDecorator([](int a, int b) { return a + b; })
    .withHooks(
        []() { std::cout << "Before execution"; },
        [](int result) { std::cout << "Result: " << result; },
        [](auto duration) { std::cout << "Execution time: " << duration.count() << "µs"; }
    );
int result = decoratedFunc(3, 4);
```

### LoopDecorator

A decorator that executes a function multiple times.

```cpp
template <typename FuncType>
struct LoopDecorator : public decorator<FuncType> {
    // ... implementation ...
};
```

#### Usage:

```cpp
auto loopDecorator = makeLoopDecorator([](int a) { std::cout << a << " "; });
loopDecorator(3, 5);  // Prints "5 5 5 "
```

### ConditionCheckDecorator

A decorator that executes a function only if a condition is met.

```cpp
template <typename FuncType>
struct ConditionCheckDecorator : public decorator<FuncType> {
    // ... implementation ...
};
```

#### Usage:

```cpp
auto conditionDecorator = makeConditionCheckDecorator([](int a) { std::cout << a; });
conditionDecorator([]() { return true; }, 5);  // Prints "5"
conditionDecorator([]() { return false; }, 5);  // Prints nothing
```

### BaseDecorator

An abstract base class for creating custom decorators.

```cpp
template <typename R, typename... Args>
class BaseDecorator {
    // ... implementation ...
};
```

### DecorateStepper

A class that allows chaining multiple decorators.

```cpp
template <typename R, typename... Args>
class DecorateStepper {
    // ... implementation ...
};
```

#### Usage:

```cpp
auto stepper = makeDecorateStepper([](int a, int b) { return a + b; });
stepper.addDecorator<MyCustomDecorator>();
stepper.addDecorator<AnotherDecorator>();
int result = stepper.execute(3, 4);
```

## Detailed Descriptions

### Switchable

The `Switchable` class allows dynamic switching between different implementations of a function. It uses `std::function` to store the current implementation and provides a `switchTo` method to change the implementation at runtime.

### decorator

The `decorator` class template is the core of the decoration system. It allows adding three types of hooks to a function:

1. Before execution (`before_`)
2. After execution (`callback_`)
3. After execution with timing information (`after_`)

The `withHooks` method is used to set these hooks. The decorated function can then be called like a normal function, and the hooks will be executed at the appropriate times.

### LoopDecorator

The `LoopDecorator` extends the base `decorator` class to add looping functionality. It takes an additional `loopCount` parameter when called, which determines how many times the decorated function will be executed.

### ConditionCheckDecorator

The `ConditionCheckDecorator` extends the base `decorator` class to add conditional execution. It takes an additional condition function when called, which determines whether the decorated function should be executed.

### BaseDecorator

The `BaseDecorator` class provides an interface for creating custom decorators. Custom decorators should inherit from this class and implement the `operator()` method.

### DecorateStepper

The `DecorateStepper` class allows chaining multiple decorators. Decorators are applied in the reverse order they are added (last added is executed first). The `execute` method applies all decorators and then calls the base function.

## Error Handling

The system includes a custom `DecoratorError` class for handling decorator-specific errors. The `DecorateStepper::execute` method catches and rethrows these errors.

## Performance Considerations

- The system uses `std::function` for type erasure, which may have some performance overhead.
- The `LoopDecorator` uses `#pragma unroll` for potential loop unrolling optimizations.
- The `DecorateStepper` applies decorators in reverse order, which may affect performance for a large number of decorators.

## Thread Safety

The decorator system does not provide built-in thread safety. If used in a multi-threaded environment, external synchronization should be applied.

## Extensibility

The system is designed to be extensible:

- New decorator types can be created by inheriting from `BaseDecorator`.
- Custom decorators can be added to a `DecorateStepper` using the `addDecorator` method.

## Notes

- The system heavily relies on C++17 features like `if constexpr` and `std::invoke`.
- It supports both value semantics and reference semantics for function arguments.
- The `Callable` concept is used to ensure type safety when switching function implementations.
- The `FunctionTraits` template (not shown in this file) is used to deduce function signature information.

## Helper Functions

- `makeDecorator`: Creates a `decorator` instance from a function.
- `makeLoopDecorator`: Creates a `LoopDecorator` instance from a function.
- `makeConditionCheckDecorator`: Creates a `ConditionCheckDecorator` instance from a function.
- `makeDecorateStepper`: Creates a `DecorateStepper` instance from a function.

These helper functions simplify the creation and use of decorators.
