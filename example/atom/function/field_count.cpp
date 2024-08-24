/*!
 * \file field_count_examples.cpp
 * \brief Examples of using Field Count functionality.
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/field_count.hpp"

#include <array>
#include <iostream>

// Define some example structs with varying numbers of fields
struct EmptyStruct {};

struct SingleField {
    int a;
};

struct MultipleFields {
    int a;
    double b;
    std::string c;
};

// Define a type trait to provide field count for structs
template <>
struct atom::meta::TypeInfo<MultipleFields> {
    static constexpr std::size_t count = 3;
};

// Define an array for testing
constexpr std::array<int, 5> intArray = {1, 2, 3, 4, 5};

// Define a non-aggregate type for testing
class NonAggregate {
public:
    NonAggregate() = default;
    void method() {}
};

// Function to demonstrate field count for different types
void demoFieldCount() {
    using namespace atom::meta;

    // Field count for an empty struct
    constexpr auto emptyCount = fieldCountOf<EmptyStruct>();
    std::cout << "Field count of EmptyStruct: " << emptyCount << std::endl;

    // Field count for a struct with a single field
    constexpr auto singleFieldCount = fieldCountOf<SingleField>();
    std::cout << "Field count of SingleField: " << singleFieldCount
              << std::endl;

    // Field count for a struct with multiple fields
    constexpr auto multipleFieldsCount = fieldCountOf<MultipleFields>();
    std::cout << "Field count of MultipleFields: " << multipleFieldsCount
              << std::endl;

    // Field count for an array
    constexpr auto arrayFieldCount = fieldCountOf<decltype(intArray)>();
    std::cout << "Field count of intArray: " << arrayFieldCount << std::endl;

    // Field count for a non-aggregate type (should be 0)
    constexpr auto nonAggregateCount = fieldCountOf<NonAggregate>();
    std::cout << "Field count of NonAggregate: " << nonAggregateCount
              << std::endl;
}

int main() {
    demoFieldCount();
    return 0;
}
