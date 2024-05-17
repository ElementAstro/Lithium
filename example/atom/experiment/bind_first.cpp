#include "atom/experiment/bind_first.hpp"

#include <iostream>
#include <string>

int add(int a, int b) { return a + b; }

struct Adder {
    int operator()(int a, int b) const { return a + b; }
};

struct Person {
    std::string name;

    void greet(const std::string &message) const {
        std::cout << "Hello, " << name << "! " << message << std::endl;
    }
};

int main() {
    // 绑定普通函数
    auto add5 = bind_first(add, 5);
    std::cout << add5(3) << std::endl;  // 输出: 8

    // 绑定函数对象
    // Adder adder;
    // auto adder10 = bind_first(adder, 10);
    // std::cout << adder10(7) << std::endl;  // 输出: 17

    // 绑定成员函数
    Person person{"Alice"};
    auto greet_alice = bind_first(&Person::greet, person);
    greet_alice("How are you?");  // 输出: Hello, Alice! How are you?

    // 绑定 std::function
    std::function<int(int, int)> fn = [](int a, int b) { return a * b; };
    auto multiply3 = bind_first(fn, 3);
    std::cout << multiply3(5) << std::endl;  // 输出: 15

    // 绑定成员函数对象
    // auto greet_bob = bind_first(std::cref(person), "Bob", &Person::greet);
    // greet_bob("Nice to meet you!");  // 输出: Hello, Bob! Nice to meet you!

    // 使用 is_invocable_v 和 is_nothrow_invocable_v
    static_assert(is_invocable_v<decltype(add5), int>);
    // static_assert(is_nothrow_invocable_v<decltype(adder10), int>);
    static_assert(is_invocable_v<decltype(greet_alice), const std::string &>);
    static_assert(is_invocable_v<decltype(multiply3), int>);
    // static_assert(is_invocable_v<decltype(greet_bob), const std::string &>);

    return 0;
}
