#include "atom/experiment/invoke.hpp"

#include <iostream>

void print_message(const std::string &message)
{
    std::cout << message << std::endl;
}

int add_numbers(int a, int b)
{
    return a + b;
}

int main()
{
    auto delayed_print = delay_invoke(print_message, "Hello, World!");
    auto delayed_add = delay_invoke(add_numbers, 10, 20);

    // 延迟调用print_message函数
    delayed_print();

    // 延迟调用add_numbers函数并获取结果
    int result = delayed_add();
    std::cout << "Result: " << result << std::endl;

    return 0;
}
