/*
 * optional.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A simple implementation of optional. Using C++11.

**************************************************/

#ifndef ATOM_EXPERIMENTAL_OPTIONAL_HPP
#define ATOM_EXPERIMENTAL_OPTIONAL_HPP

#include <iostream>
#include <stdexcept>

template <typename T>
class Optional
{
private:
    alignas(T) unsigned char storage[sizeof(T)];
    bool hasValue;

public:
    Optional() : hasValue(false) {}

    Optional(const T &value)
    {
        new (storage) T(value);
        hasValue = true;
    }

    Optional(T &&value)
    {
        new (storage) T(std::move(value));
        hasValue = true;
    }

    Optional(const Optional &other)
    {
        if (other.hasValue)
        {
            new (storage) T(other.value());
            hasValue = true;
        }
        else
        {
            hasValue = false;
        }
    }

    Optional(Optional &&other)
    {
        if (other.hasValue)
        {
            new (storage) T(std::move(other.value()));
            hasValue = true;
        }
        else
        {
            hasValue = false;
        }
    }

    ~Optional()
    {
        reset();
    }

    Optional &operator=(const Optional &other)
    {
        if (this != &other)
        {
            reset();
            if (other.hasValue)
            {
                new (storage) T(*other);
                hasValue = true;
            }
            else
            {
                hasValue = false;
            }
        }
        return *this;
    }

    Optional &operator=(Optional &&other)
    {
        if (this != &other)
        {
            reset();
            if (other.hasValue)
            {
                new (storage) T(std::move(*other));
                hasValue = true;
            }
            else
            {
                hasValue = false;
            }
        }
        return *this;
    }

    T &operator*()
    {
        if (!hasValue)
        {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<T *>(storage);
    }

    const T &operator*() const
    {
        if (!hasValue)
        {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<const T *>(storage);
    }

    void reset()
    {
        if (hasValue)
        {
            reinterpret_cast<T *>(storage)->~T();
            hasValue = false;
        }
    }

    T &value()
    {
        if (!hasValue)
        {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<T *>(storage);
    }

    const T &value() const
    {
        if (!hasValue)
        {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<const T *>(storage);
    }

    explicit operator bool() const
    {
        return hasValue;
    }

    bool operator==(const Optional &other) const
    {
        if (hasValue != other.hasValue)
        {
            return false;
        }
        if (hasValue)
        {
            return value() == other.value();
        }
        return true;
    }

    bool operator!=(const Optional &other) const
    {
        return !(*this == other);
    }

    T value_or(const T &defaultValue) const
    {
        return hasValue ? value() : defaultValue;
    }
};

#endif
