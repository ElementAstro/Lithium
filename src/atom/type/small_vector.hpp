/*
 * small_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-17

Description: A Small Vector Implementation

**************************************************/

#ifndef ATOM_TYPE_SMALL_VECTOR_HPP
#define ATOM_TYPE_SMALL_VECTOR_HPP

#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

// 内部存储的最大容量
constexpr std::size_t InternalBufferSize = 16;

template <typename T, std::size_t N = InternalBufferSize>
class SmallVector {
public:
    SmallVector() noexcept : m_size(0), m_isExternal(false) {}

    SmallVector(std::initializer_list<T> ilist) : SmallVector() {
        reserve(ilist.size());
        std::uninitialized_copy(ilist.begin(), ilist.end(),
                                m_isExternal ? m_external : m_internal);
        m_size = ilist.size();
    }

    SmallVector(const SmallVector& other) : SmallVector() {
        reserve(other.m_size);
        std::uninitialized_copy(other.begin(), other.end(),
                                m_isExternal ? m_external : m_internal);
        m_size = other.m_size;
    }

    SmallVector(SmallVector&& other) noexcept : SmallVector() {
        moveAssign(std::move(other));
    }

    ~SmallVector() { clear(); }

    SmallVector& operator=(const SmallVector& other) {
        if (this != &other) {
            clear();
            reserve(other.m_size);
            std::uninitialized_copy(other.begin(), other.end(),
                                    m_isExternal ? m_external : m_internal);
            m_size = other.m_size;
        }
        return *this;
    }

    SmallVector& operator=(SmallVector&& other) noexcept {
        if (this != &other) {
            moveAssign(std::move(other));
        }
        return *this;
    }

    T& operator[](std::size_t index) {
        return const_cast<T&>(static_cast<const SmallVector&>(*this)[index]);
    }

    const T& operator[](std::size_t index) const {
        return m_isExternal ? m_external[index] : m_internal[index];
    }

    bool operator==(const SmallVector& other) const {
        if (m_size != other.m_size) {
            return false;
        }
        for (std::size_t i = 0; i < m_size; ++i) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SmallVector& other) const {
        return !(*this == other);
    }

    T* data() noexcept { return m_isExternal ? m_external : m_internal; }

    const T* data() const noexcept {
        return m_isExternal ? m_external : m_internal;
    }

    std::size_t size() const noexcept { return m_size; }

    std::size_t capacity() const noexcept {
        return m_isExternal ? m_externalCapacity : N;
    }

    bool empty() const noexcept { return m_size == 0; }

    void clear() noexcept {
        if (m_isExternal) {
            std::destroy(m_external, m_external + m_size);
            ::operator delete(m_external, m_externalCapacity * sizeof(T));
            m_external = nullptr;
            m_externalCapacity = 0;
        } else {
            std::destroy(m_internal, m_internal + m_size);
        }
        m_size = 0;
        m_isExternal = false;
    }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity()) {
            return;
        }

        if (newCapacity > N) {
            if (m_isExternal) {
                m_external =
                    static_cast<T*>(::operator new(newCapacity * sizeof(T)));
                std::uninitialized_copy(
                    std::make_move_iterator(m_external),
                    std::make_move_iterator(m_external + m_size), m_external);
                std::destroy(m_external, m_external + m_size);
                ::operator delete(m_external, m_externalCapacity * sizeof(T));
                m_externalCapacity = newCapacity;
            } else {
                T* newBuffer =
                    static_cast<T*>(::operator new(newCapacity * sizeof(T)));
                std::uninitialized_copy(
                    std::make_move_iterator(m_internal),
                    std::make_move_iterator(m_internal + m_size), newBuffer);
                std::destroy(m_internal, m_internal + m_size);
                m_external = newBuffer;
                m_externalCapacity = newCapacity;
                m_isExternal = true;
            }
        }
    }

    void resize(std::size_t newSize) {
        reserve(newSize);
        if (newSize > m_size) {
            std::uninitialized_fill(
                m_isExternal ? m_external + m_size : m_internal + m_size,
                m_isExternal ? m_external + newSize : m_internal + newSize,
                T());
        } else {
            std::destroy(
                m_isExternal ? m_external + newSize : m_internal + newSize,
                m_isExternal ? m_external + m_size : m_internal + m_size);
        }
        m_size = newSize;
    }

    void push_back(const T& value) {
        reserve(m_size + 1);
        ::new (m_isExternal ? m_external + m_size : m_internal + m_size)
            T(value);
        ++m_size;
    }

    void push_back(T&& value) {
        reserve(m_size + 1);
        ::new (m_isExternal ? m_external + m_size : m_internal + m_size)
            T(std::move(value));
        ++m_size;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        reserve(m_size + 1);
        T* newElement =
            m_isExternal ? m_external + m_size : m_internal + m_size;
        ::new (newElement) T(std::forward<Args>(args)...);
        ++m_size;
        return *newElement;
    }

    void pop_back() {
        --m_size;
        std::destroy_at(m_isExternal ? m_external + m_size
                                     : m_internal + m_size);
    }

public:
    // 迭代器类型别名
    using iterator = T*;
    using const_iterator = const T*;

    // 迭代器方法
    iterator begin() noexcept { return data(); }

    const_iterator begin() const noexcept { return data(); }

    iterator end() noexcept { return data() + m_size; }

    const_iterator end() const noexcept { return data() + m_size; }

    const_iterator cbegin() const noexcept { return begin(); }

    const_iterator cend() const noexcept { return end(); }

    // 反向迭代器方法
    std::reverse_iterator<iterator> rbegin() noexcept {
        return std::reverse_iterator<iterator>(end());
    }

    std::reverse_iterator<const_iterator> rbegin() const noexcept {
        return std::reverse_iterator<const_iterator>(end());
    }

    std::reverse_iterator<iterator> rend() noexcept {
        return std::reverse_iterator<iterator>(begin());
    }

    std::reverse_iterator<const_iterator> rend() const noexcept {
        return std::reverse_iterator<const_iterator>(begin());
    }

    std::reverse_iterator<const_iterator> crbegin() const noexcept {
        return rbegin();
    }

    std::reverse_iterator<const_iterator> crend() const noexcept {
        return rend();
    }

public:
    // 三路比较运算符
public:
    /*
    // 三路比较运算符
        auto operator<=>(const SmallVector& other) const {
            if constexpr (std::is_arithmetic_v<T>) {
                // 对于内置算术类型,使用 std::is_eq 和 std::is_neq
                for (std::size_t i = 0; i < std::min(m_size, other.m_size); ++i)
    { if (std::is_neq((*this)[i], other[i])) { return (*this)[i] < other[i] ?
    std::partial_ordering::less : (*this)[i] > other[i] ?
    std::partial_ordering::greater : std::partial_ordering::unordered;
                    }
                }

                return m_size < other.m_size   ? std::partial_ordering::less
                       : m_size > other.m_size ? std::partial_ordering::greater
                                               :
    std::partial_ordering::equivalent; } else {
                // 对于自定义类型,使用 operator<=>
                using BuiltinThreeWay =
                    std::partial_ordering (*)(const T&, const T&);
                BuiltinThreeWay cmp = &T::operator<=>;

                for (std::size_t i = 0; i < std::min(m_size, other.m_size); ++i)
    { auto result = cmp((*this)[i], other[i]); if (result !=
    std::partial_ordering::equivalent) { return result;
                    }
                }

                return m_size < other.m_size   ? std::partial_ordering::less
                       : m_size > other.m_size ? std::partial_ordering::greater
                                               :
    std::partial_ordering::equivalent;
            }
        }
    */

public:
    static std::string partialOrderingToString(std::partial_ordering order) {
        switch (order) {
            case std::partial_ordering::less:
                return "less";
            case std::partial_ordering::equivalent:
                return "equivalent";
            case std::partial_ordering::greater:
                return "greater";
            case std::partial_ordering::unordered:
                return "unordered";
            default:
                return "unknown";
        }
    }

private:
    void moveAssign(SmallVector&& other) noexcept {
        clear();
        if (other.m_isExternal) {
            m_external = other.m_external;
            m_externalCapacity = other.m_externalCapacity;
            other.m_external = nullptr;
            other.m_externalCapacity = 0;
        } else {
            std::uninitialized_copy(
                std::make_move_iterator(other.m_internal),
                std::make_move_iterator(other.m_internal + other.m_size),
                m_internal);
        }
        m_size = other.m_size;
        m_isExternal = other.m_isExternal;
        other.m_size = 0;
        other.m_isExternal = false;
    }

    alignas(T) std::byte m_internalBuffer[N * sizeof(T)];
    T* m_internal = reinterpret_cast<T*>(m_internalBuffer);
    T* m_external = nullptr;
    std::size_t m_externalCapacity = 0;
    std::size_t m_size = 0;
    bool m_isExternal = false;
};

#endif
