/*!
 * \file proxy_params_examples.cpp
 * \brief Examples demonstrating the use of FunctionParams class from
 * proxy_params.hpp \author Max Qian <lightapt.com> \date 2024-08-23 \copyright
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/proxy_params.hpp"

#include <any>
#include <iostream>
#include <string>
#include <vector>

// Function to demonstrate various operations on FunctionParams
void demonstrateFunctionParams() {
    // Constructing FunctionParams with different methods

    // Using a single std::any value
    FunctionParams fp1(std::any(42));

    // Using an initializer list
    FunctionParams fp2{42, std::string("Hello"), 3.14};

    // Using a vector of std::any
    std::vector<std::any> vec = {42, std::string("World"), 2.71};
    FunctionParams fp3(vec);

    // Accessing elements
    std::cout << "fp2[0]: " << std::any_cast<int>(fp2[0]) << "\n";
    std::cout << "fp2[1]: " << std::any_cast<std::string>(fp2[1]) << "\n";
    std::cout << "fp2[2]: " << std::any_cast<double>(fp2[2]) << "\n";

    // Using get method to safely access elements
    auto value1 = fp2.get<int>(0);
    auto value2 = fp2.get<std::string>(1);
    auto value3 = fp2.get<double>(2);

    std::cout << "fp2.get<int>(0): "
              << (value1 ? std::to_string(*value1) : "nullopt") << "\n";
    std::cout << "fp2.get<std::string>(1): " << (value2 ? *value2 : "nullopt")
              << "\n";
    std::cout << "fp2.get<double>(2): "
              << (value3 ? std::to_string(*value3) : "nullopt") << "\n";

    // Slicing
    auto slice = fp2.slice(1, 3);
    std::cout << "Sliced params:\n";
    for (std::size_t i = 0; i < slice.size(); ++i) {
        if (i == 0)
            std::cout << "slice[0]: " << std::any_cast<std::string>(slice[i])
                      << "\n";
        if (i == 1)
            std::cout << "slice[1]: " << std::any_cast<double>(slice[i])
                      << "\n";
    }

    // Filtering
    auto filtered = fp2.filter([](const std::any& a) {
        return a.type() == typeid(int) && std::any_cast<int>(a) > 40;
    });
    std::cout << "Filtered params (int > 40):\n";
    for (const auto& elem : filtered) {
        std::cout << std::any_cast<int>(elem) << "\n";
    }

    // Modifying elements
    fp2.set(0, 99);
    std::cout << "Modified fp2[0]: " << std::any_cast<int>(fp2[0]) << "\n";

    // Attempt to access an out-of-range index
    try {
        std::cout << "Out of range access: " << std::any_cast<int>(fp2[10])
                  << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

int main() {
    demonstrateFunctionParams();
    return 0;
}
