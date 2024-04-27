#include "atom/experiment/callable.hpp"

#include <iostream>
#include <string>

struct Person {
    Person(std::string name, int age) : m_name(std::move(name)), m_age(age) {}

    void greet(const std::string &message) const {
        std::cout << "Hello, " << m_name << "! " << message << std::endl;
    }

    std::string m_name;
    int m_age;
};

int add(int a, int b) { return a + b; }

int main() {
    // 使用 Constructor 构造对象
    Constructor<Person, std::string, int> person_constructor;
    auto person = person_constructor("Alice", 30);
    std::cout << "Name: " << person->m_name << ", Age: " << person->m_age
              << std::endl;

    // 使用 Const_Caller 调用常成员函数
    Const_Caller<void, Person, const std::string &> greet_caller(
        &Person::greet);
    greet_caller(*person, "How are you?");

    // 使用 Fun_Caller 调用普通函数
    Fun_Caller<int, int, int> add_caller(add);
    int sum = add_caller(3, 5);
    std::cout << "Sum: " << sum << std::endl;

    // 使用 Caller 调用非常成员函数
    struct Square {
        int operator()(int x) { return x * x; }
    };
    Square square;
    Caller<int, Square, int> square_caller(&Square::operator());
    int result = square_caller(square, 4);
    std::cout << "Square: " << result << std::endl;

    // 使用 Arity 获取函数参数个数
    // static_assert(Arity<void(int, double, std::string)>::value == 3);

    // 使用 Function_Signature 获取函数签名
    static_assert(
        std::is_same_v<Function_Signature<int(double, char)>::Return_Type,
                       int>);
    static_assert(
        std::is_same_v<Function_Signature<int(double, char)>::Signature,
                       int (*)(double, char)>);

    // 使用 Callable_Traits 获取可调用对象的特征
    auto lambda = [](int x, int y) { return x + y; };
    static_assert(
        std::is_same_v<Callable_Traits<decltype(lambda)>::Return_Type, int>);
    static_assert(std::is_same_v<Callable_Traits<decltype(lambda)>::Signature,
                                 int (*)(int, int)>);

    return 0;
}