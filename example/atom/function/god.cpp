/*!
 * \file god.cpp
 * \brief Examples demonstrating the use of functions and type traits from
 * god.hpp \author Max Qian <lightapt.com> \date 2024-08-23 \copyright Copyright
 * (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/god.hpp"

#include <cstring>
#include <iostream>
#include <vector>

using namespace atom::meta;

// Function to demonstrate alignment functions
void demonstrateAlignment() {
    constexpr std::size_t alignment = 16;

    std::size_t value = 15;
    std::cout << "Original value: " << value << "\n";
    std::cout << "Align up to " << alignment << ": "
              << alignUp<alignment>(value) << "\n";
    std::cout << "Align down to " << alignment << ": "
              << alignDown<alignment>(value) << "\n";

    // Align pointers
    int array[10];
    int* ptr = array;
    std::cout << "Original pointer: " << static_cast<void*>(ptr) << "\n";
    std::cout << "Aligned up pointer: "
              << static_cast<void*>(alignUp<alignment>(ptr)) << "\n";
    std::cout << "Aligned down pointer: "
              << static_cast<void*>(alignDown<alignment>(ptr)) << "\n";
}

// Function to demonstrate arithmetic operations
void demonstrateArithmeticOperations() {
    int value = 10;
    std::cout << "Original value: " << value << "\n";
    std::cout << "After fetchAdd(5): " << fetchAdd(&value, 5) << "\n";
    std::cout << "After fetchSub(3): " << fetchSub(&value, 3) << "\n";
    std::cout << "After fetchAnd(6): " << fetchAnd(&value, 6) << "\n";
    std::cout << "After fetchOr(4): " << fetchOr(&value, 4) << "\n";
    std::cout << "After fetchXor(2): " << fetchXor(&value, 2) << "\n";
}

// Function to demonstrate type traits
void demonstrateTypeTraits() {
    std::cout << "isSame<int, int>: " << std::boolalpha << isSame<int, int>()
              << "\n";
    std::cout << "isSame<int, double, int>: " << std::boolalpha
              << isSame<int, double, int>() << "\n";

    std::cout << "isRef<int>: " << std::boolalpha << isRef<int>() << "\n";
    std::cout << "isRef<int&>: " << std::boolalpha << isRef<int&>() << "\n";

    std::cout << "isArray<int[10]>: " << std::boolalpha << isArray<int[10]>()
              << "\n";
    std::cout << "isArray<int>: " << std::boolalpha << isArray<int>() << "\n";

    std::cout << "isClass<std::vector<int>>: " << std::boolalpha
              << isClass<std::vector<int>>() << "\n";
    std::cout << "isClass<int>: " << std::boolalpha << isClass<int>() << "\n";

    std::cout << "isScalar<int>: " << std::boolalpha << isScalar<int>() << "\n";
    std::cout << "isScalar<std::vector<int>>: " << std::boolalpha
              << isScalar<std::vector<int>>() << "\n";

    std::cout << "isTriviallyCopyable<int>: " << std::boolalpha
              << isTriviallyCopyable<int>() << "\n";
    std::cout << "isTriviallyCopyable<std::vector<int>>: " << std::boolalpha
              << isTriviallyCopyable<std::vector<int>>() << "\n";

    std::cout << "isTriviallyDestructible<int>: " << std::boolalpha
              << isTriviallyDestructible<int>() << "\n";
    std::cout << "isTriviallyDestructible<std::vector<int>>: " << std::boolalpha
              << isTriviallyDestructible<std::vector<int>>() << "\n";

    std::cout << "isBaseOf<std::vector<int>, std::allocator<int>>: "
              << std::boolalpha
              << isBaseOf<std::allocator<int>, std::vector<int>>() << "\n";
    std::cout << "isBaseOf<std::allocator<int>, std::vector<int>>: "
              << std::boolalpha
              << isBaseOf<std::allocator<int>, std::vector<int>>() << "\n";

    std::cout << "hasVirtualDestructor<std::vector<int>>: " << std::boolalpha
              << hasVirtualDestructor<std::vector<int>>() << "\n";
}

int main() {
    std::cout << "Demonstrating Alignment Functions:\n";
    demonstrateAlignment();

    std::cout << "\nDemonstrating Arithmetic Operations:\n";
    demonstrateArithmeticOperations();

    std::cout << "\nDemonstrating Type Traits:\n";
    demonstrateTypeTraits();

    return 0;
}
