/*!
 * \file decorate_examples.cpp
 * \brief Examples of using the decorate functionality.
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/decorate.hpp"

#include <iostream>
#include <string>

// Example function to be decorated
int add(int a, int b) { return a + b; }

void printHello() { std::cout << "Hello!" << std::endl; }

void printGoodbye() { std::cout << "Goodbye!" << std::endl; }

std::string greet(const std::string& name) { return "Hello, " + name + "!"; }

// Main function showcasing different decorators
int main() {
    // Example 1: Basic decorator usage
    auto decoratedAdd = atom::meta::makeDecorator([](int a, int b) -> int {
        std::cout << "Before addition" << std::endl;
        int result = add(a, b);
        std::cout << "After addition: " << result << std::endl;
        return result;
    });

    // Usage of the basic decorator
    int result = decoratedAdd(3, 4);
    std::cout << "Result: " << result << std::endl;

    // Example 2: LoopDecorator usage
    auto loopedAdd = atom::meta::makeLoopDecorator(
        [](int a, int b) -> int { return a + b; });

    int loopCount = 5;
    int loopedResult = loopedAdd(loopCount, 1, 2);
    std::cout << "Looped result: " << loopedResult << std::endl;

    // Example 3: ConditionCheckDecorator usage
    auto conditionCheckedGreet = atom::meta::makeConditionCheckDecorator(
        [](const std::string& name) -> std::string {
            return "Hello, " + name + "!";
        });

    bool condition = true;
    std::string greeting =
        conditionCheckedGreet([condition]() { return condition; }, "Alice");
    std::cout << greeting << std::endl;

    // Example 4: Using DecorateStepper to combine decorators
    auto stepper = atom::meta::makeDecorateStepper(
        [](int a, int b) -> int { return a + b; });

    // Adding decorators
    stepper.addDecorator(
        atom::meta::makeDecorator([](auto&& func, int a, int b) -> int {
            std::cout << "Before call" << std::endl;
            int result = func(a, b);
            std::cout << "After call: " << result << std::endl;
            return result;
        }));

    stepper.addDecorator(atom::meta::makeLoopDecorator(
        [](int a, int b) -> int { return a + b; }));

    // Executing the decorated function
    int stepperResult = stepper.execute(5, 3);
    std::cout << "Stepper result: " << stepperResult << std::endl;

    return 0;
}
