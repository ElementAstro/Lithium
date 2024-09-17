#include <iostream>
#include <string>

#include "atom/type/argsview.hpp"

int main() {
    // Example 1: Creating an ArgsView and accessing elements
    ArgsView<int, double, std::string> argsView(42, 3.14, "Hello, World!");
    std::cout << "First element: " << argsView.get<0>() << "\n";
    std::cout << "Second element: " << argsView.get<1>() << "\n";
    std::cout << "Third element: " << argsView.get<2>() << "\n";

    // Example 2: Using forEach to print all elements
    std::cout << "All elements: ";
    argsView.forEach([](const auto& arg) { std::cout << arg << " "; });
    std::cout << "\n";

    // Example 3: Transforming elements
    auto transformedView = argsView.transform([](const auto& arg) {
        if constexpr (std::is_same_v<decltype(arg), int>) {
            return arg * 2;
        } else if constexpr (std::is_same_v<decltype(arg), double>) {
            return arg + 1.0;
        } else if constexpr (std::is_same_v<decltype(arg), std::string>) {
            return arg + "!!!";
        }
    });

    std::cout << "Transformed elements: ";
    transformedView.forEach([](const auto& arg) { std::cout << arg << " "; });
    std::cout << "\n";

    // Example 4: Accumulating elements
    int sum = argsView.accumulate(
        [](int acc, const auto& arg) {
            if constexpr (std::is_arithmetic_v<decltype(arg)>) {
                return acc + arg;
            } else {
                return acc;
            }
        },
        0);
    std::cout << "Sum of numeric elements: " << sum << "\n";

    // Example 5: Using apply to call a function with all elements
    auto concatenated = std::apply(
        [](const auto&... args) { return (std::to_string(args) + ...); },
        argsView.toTuple());
    std::cout << "Concatenated elements: " << concatenated << "\n";

    // Example 6: Using makeArgsView to create an ArgsView
    auto argsView2 = makeArgsView(1, 2.5, "Test");
    std::cout << "ArgsView2 elements: ";
    argsView2.forEach([](const auto& arg) { std::cout << arg << " "; });
    std::cout << "\n";

    // Example 7: Using sum and concat helper functions
    int total = sum(1, 2, 3, 4, 5);
    std::cout << "Sum of 1, 2, 3, 4, 5: " << total << "\n";

    std::string concatenatedStr = concat("Hello", " ", "ArgsView", "!");
    std::cout << "Concatenated string: " << concatenatedStr << "\n";

    return 0;
}