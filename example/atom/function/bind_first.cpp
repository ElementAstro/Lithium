/*
 * main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-10-01

Description: Example usage of the bindFirst function.

**************************************************/

#include "atom/function/bind_first.hpp"

#include <iostream>

// A simple example class
class MyClass {
public:
    void display(int x, const std::string& message) {
        std::cout << "MyClass::display called with x: " << x
                  << " and message: " << message << std::endl;
    }

    int add(int a, int b) { return a + b; }
};

// A simple free function
void printMessage(float number, const std::string& message) {
    std::cout << "Message: " << message << " with number: " << number
              << std::endl;
}

int main() {
    MyClass myObj;

    // Bind a member function of MyClass
    auto boundDisplay = atom::meta::bindFirst(&MyClass::display, myObj);

    // Call the bound function
    boundDisplay(10, "Hello, World!");

    // Bind a free function
    auto boundPrintMessage = atom::meta::bindFirst(printMessage, 3.14f);

    // Call the bound free function
    boundPrintMessage("This is a test message");

    // Binding with a member function that returns a value
    auto boundAdd = atom::meta::bindFirst(&MyClass::add, myObj);

    // Call the bound add function and get the result
    int result = boundAdd(5, 7);
    std::cout << "Result of add: " << result << std::endl;

    return 0;
}
