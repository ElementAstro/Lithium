/*!
 * \file raw_name_examples.cpp
 * \brief Examples demonstrating the use of raw_name functions from raw_name.hpp
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/raw_name.hpp"

#include <iostream>
#include <string_view>

// Example enum
enum class MyEnum {
    Value1,
    Value2
};

// Example class template
template <typename T>
class MyClass {
public:
    T value;
};

// Example class with a member function
class MyClassWithMember {
public:
    void myFunction() {}
};

// Example using raw_name_of with type
void example_raw_name_of() {
    std::cout << "Type name of int: " << atom::meta::raw_name_of<int>() << "\n";
    std::cout << "Type name of MyClass<int>: " << atom::meta::raw_name_of<MyClass<int>>() << "\n";
}

// Example using raw_name_of_template with class template
void example_raw_name_of_template() {
    std::cout << "Template name of MyClass<int>: " << atom::meta::raw_name_of_template<MyClass<int>>() << "\n";
}

// Example using raw_name_of with enumerator value
void example_raw_name_of_enum() {
    std::cout << "Enum name of MyEnum::Value1: " << atom::meta::raw_name_of_enum<MyEnum::Value1>() << "\n";
}

// Example using raw_name_of_member with class member
void example_raw_name_of_member() {
#ifdef ATOM_CPP_20_SUPPORT
    std::cout << "Member name of MyClassWithMember::myFunction: " 
              << atom::meta::raw_name_of_member<atom::meta::Wrapper<MyClassWithMember::myFunction>>() << "\n";
#else
    std::cout << "raw_name_of_member requires C++20 support\n";
#endif
}

int main() {
    example_raw_name_of();
    example_raw_name_of_template();
    example_raw_name_of_enum();
    example_raw_name_of_member();
    return 0;
}
