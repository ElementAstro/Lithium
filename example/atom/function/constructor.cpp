#include "atom/function/constructor.hpp"
#include <iostream>
#include <memory>
#include <string>

class Example {
public:
    Example() { std::cout << "Default constructor called." << std::endl; }

    Example(int a, double b, const std::string& c) : a_(a), b_(b), c_(c) {
        std::cout << "Parameterized constructor called: " << a_ << ", " << b_
                  << ", " << c_ << std::endl;
    }

    Example(const Example& other) : a_(other.a_), b_(other.b_), c_(other.c_) {
        std::cout << "Copy constructor called." << std::endl;
    }

    void print() const {
        std::cout << "Values: " << a_ << ", " << b_ << ", " << c_ << std::endl;
    }

private:
    int a_ = 0;
    double b_ = 0.0;
    std::string c_ = "default";
};

int main() {
    // 使用默认构造函数
    auto default_constructor = atom::meta::defaultConstructor<Example>();
    Example example1 = default_constructor();

    // 使用带参数的构造函数
    auto param_constructor =
        atom::meta::constructorWithArgs<Example, int, double, std::string>();
    std::shared_ptr<Example> example2 =
        param_constructor(42, 3.14, "Hello, world!");

    example2->print();

    /*
    // 使用复制构造函数
    auto copy_constructor = atom::meta::constructor<Example>();
    Example example3 = copy_constructor(*example2);

    example3.print();
    */

    return 0;
}
