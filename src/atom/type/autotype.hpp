/*
 * type.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-13

Description: A simple type wrapper

**************************************************/

#ifndef ATOM_TYPE_TYPE_HPP
#define ATOM_TYPE_TYPE_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace Atom::Type {
class Any {
public:
    template <typename T>
    Any(const T &value) : m_ptr(new Derived<T>(value)) {}

    template <typename T>
    T cast() const {
        return static_cast<Derived<T> *>(m_ptr)->m_value;
    }

private:
    struct Base {
        virtual ~Base() {}
    };

    template <typename T>
    struct Derived : Base {
        explicit Derived(const T &value) : m_value(value) {}
        T m_value;
    };

    Base *m_ptr;
};

// 优化后的自动类型类模板
template <typename T>
class AutoType {
public:
    explicit AutoType(const T &value) : m_value(value) {}

    template <typename U>
    auto operator+(const AutoType<U> &other) const {
        return AutoType(m_value + other.m_value);
    }

    template <typename U>
    auto operator-(const AutoType<U> &other) const {
        return AutoType(m_value - other.m_value);
    }

    template <typename U>
    auto operator*(const AutoType<U> &other) const {
        return AutoType(m_value * other.m_value);
    }

    template <typename U>
    auto operator/(const AutoType<U> &other) const {
        return AutoType(m_value / other.m_value);
    }

    template <typename U>
    auto operator%(const AutoType<U> &other) const {
        return AutoType(m_value % other.m_value);
    }

    template <typename U>
    auto operator==(const AutoType<U> &other) const {
        return m_value == other.m_value;
    }

    template <typename U>
    auto operator!=(const AutoType<U> &other) const {
        return m_value != other.m_value;
    }

    template <typename U>
    auto operator<(const AutoType<U> &other) const {
        return m_value < other.m_value;
    }

    template <typename U>
    auto operator<=(const AutoType<U> &other) const {
        return m_value <= other.m_value;
    }

    template <typename U>
    auto operator>(const AutoType<U> &other) const {
        return m_value > other.m_value;
    }

    template <typename U>
    auto operator>=(const AutoType<U> &other) const {
        return m_value >= other.m_value;
    }

    // 省略其他运算符重载

    T m_value;  // 成员变量
};

// 辅助函数模板，用于创建AutoType对象
template <typename T>
AutoType<T> makeAutoType(const T &value) {
    return AutoType<T>(value);
}

// 元组打印类模板
template <typename Tuple, std::size_t N = std::tuple_size_v<Tuple>>
struct TuplePrinter {
    static void print(const Tuple &t) {
        if constexpr (N > 1) {
            TuplePrinter<Tuple, N - 1>::print(t);
            std::cout << ", " << std::get<N - 1>(t);
        } else {
            std::cout << std::get<0>(t);
        }
    }
};
}  // namespace Atom::Type

#endif

/*
int main()
{
    AutoType<int> a(2);
    AutoType<double> b(3.5);

    auto c = a + b; // AutoType<double>(5.5)
    auto d = a - b; // AutoType<double>(-1.5)
    auto e = a * b; // AutoType<double>(7.0)
    auto f = a / b; // AutoType<double>(0.571429)

    std::cout << std::boolalpha << (a == b) << '\n'; // false
    std::cout << std::boolalpha << (a != b) << '\n'; // true
    std::cout << std::boolalpha << (a < b) << '\n';  // true
    std::cout << std::boolalpha << (a <= b) << '\n'; // true
    std::cout << std::boolalpha << (a > b) << '\n';  // false
    std::cout << std::boolalpha << (a >= b) << '\n'; // false

    auto t = std::make_tuple(1, 2.5, "Hello");
    TuplePrinter<decltype(t)>::print(t); // 1, 2.5, Hello

    return 0;
}
*/