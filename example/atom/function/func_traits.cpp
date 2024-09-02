/*!
 * \file func_traits_examples.cpp
 * \brief Examples of using Function Traits functionality.
 * \author Max Qian <lightapt.com>
 * \date 2024-08-23
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "atom/function/func_traits.hpp"

#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>

// Regular function
int regularFunction(int, double) { return 42; }

// Member function
class MyClass {
public:
    double memberFunction(int x, double y) const { return x + y; }
    void noexceptMemberFunction(int x) noexcept {}
    int volatileMemberFunction(int x) volatile { return x; }
};

// Lambda function
auto lambdaFunction = [](int x, double y) -> double { return x * y; };

// Function object
struct Functor {
    double operator()(int x, double y) const { return x - y; }
};

void printFunctionInfo(const std::string& name, auto&& func) {
#if ENABLE_DEBUG
    atom::meta::print_function_info(name, std::forward<decltype(func)>(func));
#else
    std::cout << "Function: " << name << "\n";
    std::cout << "  Return type: " << typeid(decltype(func)).name() << "\n";
    std::cout << "  Is member function: " << std::boolalpha
              << atom::meta::is_member_function_v<decltype(func)> << "\n";
    std::cout << "  Is const member function: " << std::boolalpha
              << atom::meta::is_const_member_function_v<decltype(func)> << "\n";
    std::cout
        << "  Is volatile member function: " << std::boolalpha
        << atom::meta::is_volatile_member_function_v<decltype(func)> << "\n";
    std::cout << "  Is lvalue reference member function: " << std::boolalpha
              << atom::meta::is_lvalue_reference_member_function_v<
                     decltype(func)> << "\n";
    std::cout << "  Is rvalue reference member function: " << std::boolalpha
              << atom::meta::is_rvalue_reference_member_function_v<
                     decltype(func)> << "\n";
    std::cout << "  Is noexcept: " << std::boolalpha
              << atom::meta::is_noexcept_v<decltype(func)> << "\n";
    std::cout << "  Is variadic: " << std::boolalpha
              << atom::meta::is_variadic_v<decltype(func)> << "\n";
#endif
}

int main() {
    using namespace atom::meta;

    // Regular function
    printFunctionInfo("regularFunction", regularFunction);

    // Member function
    MyClass obj;
    printFunctionInfo("MyClass::memberFunction", &MyClass::memberFunction);
    printFunctionInfo("MyClass::noexceptMemberFunction",
                      &MyClass::noexceptMemberFunction);
    printFunctionInfo("MyClass::volatileMemberFunction",
                      &MyClass::volatileMemberFunction);

    // Lambda function
    printFunctionInfo("lambdaFunction", lambdaFunction);

    // Function object
    printFunctionInfo("Functor::operator()", Functor{});

    return 0;
}
