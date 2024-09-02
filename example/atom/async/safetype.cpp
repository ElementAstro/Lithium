#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "atom/async/safetype.hpp"

// Function to simulate pushing elements to the stack
template <typename T>
void pushToStack(atom::async::LockFreeStack<T>& stack, T value) {
    stack.push(value);
    std::cout << "Pushed: " << value << std::endl;
}

// Function to simulate popping elements from the stack
template <typename T>
void popFromStack(atom::async::LockFreeStack<T>& stack) {
    auto value = stack.pop();
    if (value) {
        std::cout << "Popped: " << *value << std::endl;
    } else {
        std::cout << "Stack is empty." << std::endl;
    }
}

int main() {
    // Create a LockFreeStack for integers
    atom::async::LockFreeStack<int> stack;

    // Create a vector for threads
    std::vector<std::thread> threads;

    // Start threads to push elements onto the stack
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(pushToStack<int>, std::ref(stack), i);
    }

    // Allow some time for all pushes to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Start threads to pop elements from the stack
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(popFromStack<int>, std::ref(stack));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    // Final stack state checks
    if (stack.empty()) {
        std::cout << "The stack is empty at the end." << std::endl;
    } else {
        std::cout << "The stack is not empty at the end." << std::endl;
    }

    return 0;
}
