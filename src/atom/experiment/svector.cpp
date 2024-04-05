#ifndef CHAISCRIPT_STACK_VECTOR_HPP_
#define CHAISCRIPT_STACK_VECTOR_HPP_

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>  // for std::forward, std::move

template <typename T, std::size_t MaxSize>
struct Stack_Vector {
    Stack_Vector() noexcept = default;
    Stack_Vector(const Stack_Vector &other) {
        m_size = other.m_size;
        for (std::size_t i = 0; i < m_size; ++i) {
            new (&(*this)[i]) T(other[i]);
        }
    }

    Stack_Vector(Stack_Vector &&other) noexcept
        : data{other.data}, m_size{other.m_size} {
        other.m_size = 0;
    }

    ~Stack_Vector() noexcept(std::is_nothrow_destructible_v<T>) {
        for (std::size_t pos = 0; pos < m_size; ++pos) {
            (*this)[pos].~T();
        }
    }

    constexpr static auto aligned_size =
        sizeof(T) + (sizeof(T) & std::alignment_of_v<T>) > 0
            ? std::alignment_of_v<T>
            : 0;

    alignas(std::alignment_of_v<T>) char data[aligned_size * MaxSize];

    [[nodiscard]] T &operator[](const std::size_t idx) noexcept {
        return *reinterpret_cast<T *>(&data + aligned_size * idx);
    }

    [[nodiscard]] const T &operator[](const std::size_t idx) const noexcept {
        return *reinterpret_cast<const T *>(&data + aligned_size * idx);
    }

    template <typename... Param>
    T &emplace_back(Param &&...param) {
        auto *p = new (&(*this)[m_size++]) T(std::forward<Param>(param)...);
        return *p;
    };

    auto size() const noexcept { return m_size; };
    auto capacity() const noexcept { return MaxSize; };
    void pop_back() noexcept(std::is_nothrow_destructible_v<T>) {
        (*this)[--m_size].~T();
    }
    void resize(std::size_t new_size) { m_size = new_size; }

    Stack_Vector &operator=(const Stack_Vector &other) {
        if (this != &other) {
            for (std::size_t i = 0; i < m_size; ++i) {
                (*this)[i].~T();
            }
            m_size = other.m_size;
            for (std::size_t i = 0; i < m_size; ++i) {
                new (&(*this)[i]) T(other[i]);
            }
        }
        return *this;
    }

    Stack_Vector &operator=(Stack_Vector &&other) noexcept {
        if (this != &other) {
            for (std::size_t i = 0; i < m_size; ++i) {
                (*this)[i].~T();
            }
            std::swap(data, other.data);
            std::swap(m_size, other.m_size);
        }
        return *this;
    }

    std::size_t m_size{0};
};

#endif  // CHAISCRIPT_STACK_VECTOR_HPP_

#include <iostream>
#include <string>

struct Foo {
    int a;
    double b;
    float c;

    Foo(int a, double b, float c) : a(a), b(b), c(c) {}

    ~Foo() { std::cout << "Destructing Foo\n"; }
};

int main() {
    Stack_Vector<Foo, 10> sv;

    sv.emplace_back(1, 2.3, 1.1);
    sv.emplace_back(4, 5.6, 1.2);

    std::cout << sv[0].a << ", " << sv[0].b << sv[0].c << "\n";
    std::cout << sv[1].a << ", " << sv[1].b << sv[1].c << "\n";

    sv.pop_back();

    std::cout << sv[0].a << ", " << sv[0].b << "\n";
}