/*!
 * \file overload_examples.cpp
 * \brief Examples demonstrating the use of OverloadCast from overload.hpp
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/overload.hpp"

#include <iostream>

// Example free functions with different signatures
int add(int a, int b) { return a + b; }

int multiply(int a, int b) { return a * b; }

// Example class with various member functions
class Calculator {
public:
    int add(int a, int b) { return a + b; }

    int subtract(int a, int b) const { return a - b; }

    int multiply(int a, int b) volatile { return a * b; }

    int divide(int a, int b) const volatile {
        if (b == 0)
            throw std::runtime_error("Division by zero");
        return a / b;
    }

    int getValue() const { return value; }

    // Member variable
    int value = 42;
};

// Test OverloadCast with free functions
void testFreeFunctionOverloadCast() {
    using namespace atom::meta;

    auto addFunc = overload_cast<int, int>{}(add);
    auto multiplyFunc = overload_cast<int, int>{}(multiply);

    std::cout << "Add result: " << addFunc(5, 3) << "\n";
    std::cout << "Multiply result: " << multiplyFunc(5, 3) << "\n";
}

// Test OverloadCast with member functions
void testMemberFunctionOverloadCast() {
    using namespace atom::meta;

    Calculator calc;

    // Non-const member function
    auto addMemFunc = overload_cast<int, int>{}(&Calculator::add);
    std::cout << "Member add result: " << (calc.*addMemFunc)(10, 5) << "\n";

    // Const member function
    auto subtractMemFunc = overload_cast<int, int>{}(&Calculator::subtract);
    std::cout << "Member subtract result: "
              << (static_cast<const Calculator&>(calc).*subtractMemFunc)(10, 5)
              << "\n";

    // Volatile member function
    auto multiplyMemFunc = overload_cast<int, int>{}(&Calculator::multiply);
    std::cout << "Member multiply result: "
              << (static_cast<volatile Calculator&>(calc).*multiplyMemFunc)(10,
                                                                            5)
              << "\n";

    // Const volatile member function
    auto divideMemFunc = overload_cast<int, int>{}(&Calculator::divide);
    try {
        std::cout << "Member divide result: "
                  << (static_cast<const volatile Calculator&>(calc).*
                      divideMemFunc)(10, 2)
                  << "\n";
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

// Test OverloadCast with member variables
void testMemberVariableOverloadCast() {
    using namespace atom::meta;

    Calculator calc;

    // Member variable
    auto valueMemVar = overload_cast<int>{}(&Calculator::value);
    std::cout << "Member value: " << (calc.*valueMemVar) << "\n";
}

int main() {
    std::cout << "Testing Free Function OverloadCast:\n";
    testFreeFunctionOverloadCast();

    std::cout << "\nTesting Member Function OverloadCast:\n";
    testMemberFunctionOverloadCast();

    std::cout << "\nTesting Member Variable OverloadCast:\n";
    testMemberVariableOverloadCast();

    return 0;
}
