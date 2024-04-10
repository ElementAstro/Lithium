/*
 * any.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: A simple implementation of any type.

**************************************************/

#ifndef ATOM_EXPERIMENT_ANY_HPP
#define ATOM_EXPERIMENT_ANY_HPP

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

class Any;

template <typename T>
concept Derived = std::is_base_of_v<std::remove_reference_t<T>, Any>;

class Any {
public:
    constexpr Any() noexcept : ptr(nullptr) {}

    template <typename T>
        requires(!Derived<T>)
    constexpr Any(T &&value)
        : ptr(new holder<std::decay_t<T>>(std::forward<T>(value))) {}

    constexpr Any(const Any &other)
        : ptr(other.ptr ? other.ptr->clone() : nullptr) {}

    constexpr Any(Any &&other) noexcept
        : ptr(std::exchange(other.ptr, nullptr)) {}

    ~Any() { delete ptr; }

    constexpr Any &operator=(const Any &other) {
        if (this != &other) {
            Any(other).swap(*this);
        }
        return *this;
    }

    constexpr Any &operator=(Any &&other) noexcept {
        if (this != &other) {
            Any(std::move(other)).swap(*this);
        }
        return *this;
    }

    template <typename T>
        requires(!Derived<T>)
    constexpr Any &operator=(T &&value) {
        Any(std::forward<T>(value)).swap(*this);
        return *this;
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return !ptr; }

    [[nodiscard]] constexpr const std::type_info &type() const noexcept {
        return ptr ? ptr->type() : typeid(void);
    }

    template <typename T>
    friend T any_cast(const Any &operand);

private:
    class placeholder {
    public:
        virtual ~placeholder() = default;
        [[nodiscard]] virtual const std::type_info &type() const noexcept = 0;
        [[nodiscard]] virtual placeholder *clone() const = 0;
        virtual void swap(placeholder &other) = 0;
    };

    template <typename T>
    class holder : public placeholder {
    public:
        constexpr holder(T &&value) : held(std::move(value)) {}

        constexpr holder(const T &value) : held(value) {}

        [[nodiscard]] const std::type_info &type() const noexcept override {
            return typeid(T);
        }

        [[nodiscard]] placeholder *clone() const override {
            return new holder<T>(held);
        }

        void swap(placeholder &other) override {
            if (auto other_holder = dynamic_cast<holder *>(&other)) {
                std::swap(held, other_holder->held);
            }
        }

        T held;
    };

    placeholder *ptr;

    constexpr void swap(Any &other) noexcept { std::swap(ptr, other.ptr); }
};

template <typename T>
T any_cast(const Any &operand) {
    if (typeid(T) != operand.type()) {
        throw std::bad_cast();
    }
    return static_cast<Any::holder<T> *>(operand.ptr)->held;
}

template <typename T>
T any_cast(Any &&operand) {
    if (typeid(T) != operand.type()) {
        throw std::bad_cast();
    }
    return static_cast<Any::holder<T> *>(std::move(operand.ptr))->held;
}

#endif
