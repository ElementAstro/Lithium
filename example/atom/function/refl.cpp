/*!
 * \file refl_examples.cpp
 * \brief Examples demonstrating the use of static reflection from refl.hpp
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/refl.hpp"

#include <iostream>
#include <string>

// Define a class with reflection metadata
struct MyClass {
    int x;
    double y;
    std::string z;

    void print() const {
        std::cout << "x: " << x << ", y: " << y << ", z: " << z << '\n';
    }
};

// Define reflection metadata for MyClass
ATOM_META_TYPEINFO(MyClass, ATOM_META_FIELD("x", &MyClass::x),
                   ATOM_META_FIELD("y", &MyClass::y),
                   ATOM_META_FIELD("z", &MyClass::z))

// Define a class with a base class
struct Base {
    int baseField;
};

struct Derived : Base {
    double derivedField;
};

// Define reflection metadata for Base
ATOM_META_TYPEINFO(Base, ATOM_META_FIELD("baseField", &Base::baseField))

// Define reflection metadata for Derived
ATOM_META_TYPEINFO(Derived,
                   ATOM_META_FIELD("derivedField", &Derived::derivedField))

// Function to print the field names and values
template <typename T>
void printFields(const T& obj) {
    using TypeInfo = atom::meta::TypeInfo<T>;
    TypeInfo::ForEachVarOf(obj, [](const auto& field, const auto& value) {
        std::cout << "Field name: " << field.name << ", Value: " << value
                  << '\n';
    });
}

// Function to find and print the field value by name
template <typename T>
void printFieldByName(const T& obj, const std::string& name) {
    using TypeInfo = atom::meta::TypeInfo<T>;
    const auto& field = TypeInfo::fields.Find(TSTR(name));
    if constexpr (std::is_same_v<decltype(field.value), void>) {
        std::cout << "Field not found: " << name << '\n';
    } else {
        std::cout << "Field name: " << field.name
                  << ", Value: " << obj.*field.value << '\n';
    }
}

int main() {
    MyClass myObject{10, 3.14, "example"};

    // Print all fields of MyClass
    std::cout << "MyClass fields:\n";
    printFields(myObject);

    // Print specific fields by name
    std::cout << "\nPrinting fields by name:\n";
    printFieldByName(myObject, "x");
    printFieldByName(myObject, "y");
    printFieldByName(myObject, "z");
    printFieldByName(myObject, "nonexistent");

    // Example with Derived class
    Derived derivedObject{42, 2.718};

    std::cout << "\nDerived class fields:\n";
    printFields(derivedObject);

    std::cout << "\nPrinting fields by name for Derived:\n";
    printFieldByName(derivedObject, "derivedField");
    printFieldByName(derivedObject, "baseField");

    return 0;
}
