/*!
 * \file invoke.cpp
 * \brief Examples demonstrating the use of invoke functions from invoke.hpp
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/invoke.hpp"

#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Example function to be used with delayInvoke
int add(int a, int b) { return a + b; }

// Member function of a class
class Calculator {
public:
    int multiply(int a, int b) const { return a * b; }

    int divide(int a, int b) {
        if (b == 0)
            throw std::runtime_error("Division by zero");
        return a / b;
    }

    // Member variable
    int value = 42;
};

// Example function to demonstrate delayInvoke
void demonstrateDelayInvoke() {
    auto delayedAdd = delayInvoke(add, 3, 4);
    std::cout << "Result of delayed add: " << delayedAdd() << "\n";
}

// Example function to demonstrate delayMemInvoke
void demonstrateDelayMemInvoke() {
    Calculator calc;
    auto delayedMultiply = delayMemInvoke(&Calculator::multiply, &calc);
    std::cout << "Result of delayed multiply: " << delayedMultiply(5, 6)
              << "\n";
}

// Example function to demonstrate delayCmemInvoke
void demonstrateDelayCmemInvoke() {
    Calculator calc;
    auto delayedDivide = delayMemInvoke(&Calculator::multiply, &calc);
    std::cout << "Result of delayed divide: " << delayedDivide(8, 2) << "\n";
}

// Example function to demonstrate delayStaticMemInvoke
void demonstrateDelayStaticMemInvoke() {
    // Static member functions are not supported in this context, so this
    // example is not valid.
}

// Example function to demonstrate delayMemberVarInvoke
void demonstrateDelayMemberVarInvoke() {
    Calculator calc;
    auto getValue = delayMemberVarInvoke(&Calculator::value, &calc);
    std::cout << "Value from member variable: " << getValue() << "\n";
}

// Example function to demonstrate safeCall
void demonstrateSafeCall() {
    auto safeDivide = [](int a, int b) -> int {
        if (b == 0)
            throw std::runtime_error("Division by zero");
        return a / b;
    };

    std::cout << "Safe divide result: " << safeCall(safeDivide, 10, 2) << "\n";
    std::cout << "Safe divide result (with exception): "
              << safeCall(safeDivide, 10, 0)
              << "\n";  // Default-constructed int (0)
}

// Example function to demonstrate safeTryCatch
void demonstrateSafeTryCatch() {
    auto riskyFunction = []() -> int {
        throw std::runtime_error("An error occurred");
        return 42;
    };

    auto result = safeTryCatch(riskyFunction);
    if (std::holds_alternative<int>(result)) {
        std::cout << "Result: " << std::get<int>(result) << "\n";
    } else {
        std::cout << "Exception caught\n";
    }
}

// Example function to demonstrate safeTryCatchOrDefault
void demonstrateSafeTryCatchOrDefault() {
    auto riskyFunction = []() -> int {
        throw std::runtime_error("An error occurred");
        return 42;
    };

    int defaultValue = -1;
    std::cout << "Result: "
              << safeTryCatchOrDefault(riskyFunction, defaultValue) << "\n";
}

// Example function to demonstrate safeTryCatchWithCustomHandler
void demonstrateSafeTryCatchWithCustomHandler() {
    auto riskyFunction = []() -> int {
        throw std::runtime_error("An error occurred");
        return 42;
    };

    auto handler = [](std::exception_ptr e) {
        try {
            if (e)
                std::rethrow_exception(e);
        } catch (const std::exception& ex) {
            std::cout << "Custom handler caught exception: " << ex.what()
                      << "\n";
        }
    };

    std::cout << "Result: "
              << safeTryCatchWithCustomHandler(riskyFunction, handler) << "\n";
}

int main() {
    std::cout << "Demonstrating Delay Invoke:\n";
    demonstrateDelayInvoke();

    std::cout << "\nDemonstrating Delay Mem Invoke:\n";
    demonstrateDelayMemInvoke();

    std::cout << "\nDemonstrating Delay Cmem Invoke:\n";
    demonstrateDelayCmemInvoke();

    std::cout << "\nDemonstrating Delay Member Var Invoke:\n";
    demonstrateDelayMemberVarInvoke();

    std::cout << "\nDemonstrating Safe Call:\n";
    demonstrateSafeCall();

    std::cout << "\nDemonstrating Safe Try Catch:\n";
    demonstrateSafeTryCatch();

    std::cout << "\nDemonstrating Safe Try Catch Or Default:\n";
    demonstrateSafeTryCatchOrDefault();

    std::cout << "\nDemonstrating Safe Try Catch With Custom Handler:\n";
    demonstrateSafeTryCatchWithCustomHandler();

    return 0;
}
