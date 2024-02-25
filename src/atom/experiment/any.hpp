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

#include <typeinfo>
#include <memory>
#include <utility>

class Any
{
public:
    Any() : ptr(nullptr) {}

    template <typename T>
    Any(const T &value) : ptr(new holder<T>(value)) {}

    Any(const Any &other) : ptr(other.ptr ? other.ptr->clone() : nullptr) {}

    Any(Any &&other) noexcept : ptr(std::move(other.ptr))
    {
        other.ptr = nullptr;
    }

    ~Any()
    {
        delete ptr;
    }

    Any &operator=(const Any &other)
    {
        if (this != &other)
        {
            Any(other).swap(*this);
        }
        return *this;
    }

    Any &operator=(Any &&other) noexcept
    {
        if (this != &other)
        {
            Any(std::move(other)).swap(*this);
        }
        return *this;
    }

    template <typename T>
    Any &operator=(const T &value)
    {
        Any(value).swap(*this);
        return *this;
    }

    bool empty() const
    {
        return !ptr;
    }

    const std::type_info &type() const
    {
        return ptr ? ptr->type() : typeid(void);
    }

    template <typename T>
    friend T any_cast(const Any &operand);

private:
    class placeholder
    {
    public:
        virtual ~placeholder() {}
        virtual const std::type_info &type() const = 0;
        virtual placeholder *clone() const = 0;
        virtual void swap(placeholder &other) = 0;
    };

    template <typename T>
    class holder : public placeholder
    {
    public:
        holder(const T &value) : held(value) {}
        const std::type_info &type() const { return typeid(T); }
        placeholder *clone() const { return new holder(held); }
        void swap(placeholder &other)
        {
            if (holder *other_holder = dynamic_cast<holder *>(&other))
            {
                std::swap(held, other_holder->held);
            }
        }

        T held;
    };

    placeholder *ptr;

    void swap(Any &other)
    {
        std::swap(ptr, other.ptr);
    }
};

template <typename T>
T any_cast(const Any &operand)
{
    if (typeid(T) != operand.type())
    {
        throw std::bad_cast();
    }
    return static_cast<Any::holder<T> *>(operand.ptr)->held;
}

#endif
