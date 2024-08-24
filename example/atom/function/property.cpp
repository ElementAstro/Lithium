/*!
 * \file property_examples.cpp
 * \brief Examples demonstrating the use of Property class and macros from
 * property.hpp \author Max Qian <lightapt.com> \date 2024-08-23 \copyright
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/property.hpp"

#include <iostream>
#include <string>

// Example class using the Property class and macros
class Example {
private:
    // Define a read-write property
    DEFINE_RW_PROPERTY(int, age);

    // Define a read-only property
    DEFINE_RO_PROPERTY(std::string, name);

    // Define a write-only property
    DEFINE_WO_PROPERTY(double, salary);

public:
    Example(int age, std::string name, double salary)
        : age_(age), name_(std::move(name)), salary_(salary) {}

    // Optional: You can define additional methods or properties here
};

int main() {
    // Create an instance of Example
    Example example(30, "Alice", 50000.0);

    // Access read-write property
    std::cout << "Initial age: " << example.age() << "\n";
    example.age() = 31;
    std::cout << "Updated age: " << example.age() << "\n";

    // Access read-only property
    std::cout << "Name: " << example.name() << "\n";

    // Access write-only property (only setting the value is possible)
    example.salary() = 55000.0;
    std::cout << "Salary updated successfully.\n";

    // Attempt to access the write-only property (will cause a compilation
    // error) std::cout << "Salary: " << example.salary() << "\n";

    // Set an onChange callback for the read-write property
    example.age().setOnChange([](const int& newValue) {
        std::cout << "Age changed to: " << newValue << "\n";
    });

    // Change the age to trigger the onChange callback
    example.age() = 32;

    return 0;
}
