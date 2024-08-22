/*
 * main.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-10-01

Description: Example usage of constructors methods.

**************************************************/

#include <iostream>
#include <memory>

#include "atom/function/constructor.hpp"

// Sample class to demonstrate constructors
class MyClass {
public:
    MyClass(int x) : value(x) {}

    void display() const { std::cout << "Value: " << value << std::endl; }

    void setValue(int x) { value = x; }

    int getValue() const { return value; }

    int value;
};

// Function to demonstrate binding member functions
void demonstrateBindMemberFunction() {
    MyClass myObject(10);

    // Bind member function
    auto memberFunc = atom::meta::bindMemberFunction(&MyClass::display);
    memberFunc(myObject, 0);

    // Bind member variable
    auto bindVar = atom::meta::bindMemberVariable(&MyClass::value);
    std::cout << "Accessed value using bound member variable: "
              << bindVar(myObject) << std::endl;
}

// Function to demonstrate building constructors
void demonstrateConstructors() {
    // Build shared constructor
    auto sharedConstructor =
        atom::meta::buildSharedConstructor<MyClass>(&MyClass::MyClass);
    auto myClassInstance = sharedConstructor(15);
    myClassInstance->display();

    // Build regular constructor
    auto copyConstructor =
        atom::meta::buildCopyConstructor<MyClass>(&MyClass::MyClass);
    MyClass copiedInstance = copyConstructor(20);
    copiedInstance.display();

    // Build default constructor
    auto defaultConstructor = atom::meta::buildDefaultConstructor<MyClass>();
    MyClass defaultInstance = defaultConstructor();
    defaultInstance.display();
}

int main() {
    // Demonstrate binding of member functions and variables
    demonstrateBindMemberFunction();

    // Demonstrate various constructors
    demonstrateConstructors();

    return 0;
}
