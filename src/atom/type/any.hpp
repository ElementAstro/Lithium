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

#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace atom::type {
class Any;

template <typename T>
concept Derived = std::is_base_of_v<std::remove_reference_t<T>, Any>;

class Any {
private:
    static constexpr std::size_t BufferSize = 64;
    static constexpr std::size_t BufferAlign = alignof(std::max_align_t);

    using Storage = std::aligned_storage_t<BufferSize, BufferAlign>;

public:
    constexpr Any() noexcept : ptr(nullptr) {}

    template <typename T>
        requires(!Derived<T> &&
                 !(sizeof(T) <= BufferSize && std::is_trivially_copyable_v<T>))
    constexpr Any(T &&value)
        : ptr(new holder<std::decay_t<T>>(std::forward<T>(value))) {}

    template <typename T>
        requires(!Derived<T> && sizeof(T) <= BufferSize &&
                 std::is_trivially_copyable_v<T>)
    constexpr Any(T &&value) {
        new (&storage) holder<std::decay_t<T>>(std::forward<T>(value));
        small = true;
    }

    constexpr Any(const Any &other)
        : ptr(other.small ? nullptr
                          : (other.ptr ? other.ptr->clone() : nullptr)),
          small(other.small) {
        if (other.small) {
            std::copy_n(reinterpret_cast<const char *>(&other.storage),
                        sizeof(Storage), reinterpret_cast<char *>(&storage));
        }
    }

    constexpr Any(Any &&other) noexcept : small(other.small) {
        if (other.small) {
            std::memcpy(&storage, &other.storage, sizeof(Storage));
        } else {
            ptr = std::exchange(other.ptr, nullptr);
        }
    }

    ~Any() {
        if (small) {
            // If the stored object has a non-trivial destructor, it needs to be
            // called manually
            if (!std::is_trivially_destructible_v<decltype(storage)>) {
                // Assuming 'holder' has a virtual destructor and a method to
                // properly destroy the contained object...
                reinterpret_cast<placeholder *>(&storage)->~placeholder();
            }
        } else {
            delete ptr;
        }
    }

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
        virtual void destroy() = 0;
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

    union {
        placeholder *ptr;
        Storage storage;
    };
    bool small = false;

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
}  // namespace atom::type

#endif
