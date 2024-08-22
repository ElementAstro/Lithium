#include <iostream>

#include "atom/function/any.hpp"

int main() {
    // Create a BoxedValue containing an integer
    atom::meta::BoxedValue intValue = atom::meta::makeBoxedValue(42);
    std::cout << "Boxed integer: " << intValue.debugString() << std::endl;

    // Create a BoxedValue containing a string
    std::string testString = "Hello, BoxedValue!";
    atom::meta::BoxedValue stringValue = atom::meta::makeBoxedValue(testString);
    std::cout << "Boxed string: " << stringValue.debugString() << std::endl;

    // Create a BoxedValue containing a vector
    std::vector<int> numbers{1, 2, 3, 4, 5};
    atom::meta::BoxedValue vectorValue = atom::meta::makeBoxedValue(numbers);
    std::cout << "Boxed vector: " << vectorValue.debugString() << std::endl;

    // Demonstrate type casting
    if (auto intPtr = intValue.tryCast<int>()) {
        std::cout << "Casted integer value: " << *intPtr << std::endl;
    } else {
        std::cout << "Failed to cast to integer." << std::endl;
    }

    if (auto stringPtr = stringValue.tryCast<std::string>()) {
        std::cout << "Casted string value: " << *stringPtr << std::endl;
    } else {
        std::cout << "Failed to cast to string." << std::endl;
    }

    // Attempt to cast to an incorrect type
    if (auto doublePtr = intValue.tryCast<double>()) {
        std::cout << "Casted double value: " << *doublePtr << std::endl;
    } else {
        std::cout << "Failed to cast integer to double." << std::endl;
    }

    // Set an attribute
    stringValue.setAttr("greeting", atom::meta::makeBoxedValue("Hi there!"));
    if (auto greeting = stringValue.getAttr("greeting"); !greeting.isNull()) {
        std::cout << "Retrieved greeting: " << greeting.debugString()
                  << std::endl;
    }

    // List all attributes
    auto attributes = stringValue.listAttrs();
    std::cout << "Attributes in stringValue:" << std::endl;
    for (const auto& attr : attributes) {
        std::cout << " - " << attr << std::endl;
    }

    // Remove the attribute
    stringValue.removeAttr("greeting");
    std::cout << "Removed 'greeting' attribute." << std::endl;

    // Checking if the attribute still exists
    if (!stringValue.hasAttr("greeting")) {
        std::cout << "Attribute 'greeting' no longer exists." << std::endl;
    }
    return 0;
}
