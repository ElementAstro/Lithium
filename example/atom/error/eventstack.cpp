#include <iostream>
#include <string>

#include "atom/error/error_stack.hpp"

// Function to simulate error insertion
void simulateErrors(atom::error::ErrorStack& errorStack) {
    errorStack.insertError("Failed to connect to the database",
                           "DatabaseModule", "connect", 25, "database.cpp");
    errorStack.insertError("Invalid user input", "UserInputModule",
                           "validateInput", 42, "user_input.cpp");
    errorStack.insertError("Connection timeout", "NetworkModule", "sendRequest",
                           15, "network.cpp");
    errorStack.insertError("Failed to read configuration file", "ConfigModule",
                           "loadConfig", 33, "config.cpp");
}

// Function to demonstrate error filtering and printing
void demonstrateErrorStack() {
    // Create an instance of ErrorStack
    atom::error::ErrorStack errorStack;

    // Simulate error occurrences
    simulateErrors(errorStack);

    // Set modules to filter out (e.g., filter out errors from the
    // DatabaseModule)
    errorStack.setFilteredModules({"DatabaseModule"});

    // Print the filtered error stack
    std::cout << "Filtered error stack (excluding DatabaseModule):"
              << std::endl;
    errorStack.printFilteredErrorStack();

    // Clear the filtered modules for future prints
    errorStack.clearFilteredModules();

    // Print all errors
    std::cout << "\nAll errors in the stack:" << std::endl;
    errorStack.printFilteredErrorStack();
}

int main() {
    demonstrateErrorStack();
    return 0;
}
