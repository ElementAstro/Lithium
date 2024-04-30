/*
 * stack_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-2

Description: A simple stack vector implementation

**************************************************/

#ifndef ATOM_EXPERIMENT_STACK_VECTOR_HPP
#define ATOM_EXPERIMENT_STACK_VECTOR_HPP

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

/**
 * @brief A stack-allocated vector that stores elements of type T with a maximum
 * capacity.
 *
 * @tparam T The type of elements stored in the Stack_Vector.
 * @tparam MaxSize The maximum capacity of the Stack_Vector.
 */
template <typename T, std::size_t MaxSize>
struct Stack_Vector {
    /**
     * @brief Default constructor.
     */
    Stack_Vector() noexcept = default;

    /**
     * @brief Copy constructor.
     *
     * @param other The Stack_Vector to copy from.
     */
    Stack_Vector(const Stack_Vector &other) {
        m_size = other.m_size;
        for (std::size_t i = 0; i < m_size; ++i) {
            new (&(*this)[i]) T(other[i]);
        }
    }

    /**
     * @brief Move constructor.
     *
     * @param other The Stack_Vector to move from.
     */
    Stack_Vector(Stack_Vector &&other) noexcept
        : data{other.data}, m_size{other.m_size} {
        other.m_size = 0;
    }

    /**
     * @brief Destructor.
     */
    ~Stack_Vector() noexcept(std::is_nothrow_destructible_v<T>) {
        for (std::size_t pos = 0; pos < m_size; ++pos) {
            (*this)[pos].~T();
        }
    }

    /**
     * @brief Operator to access elements by index.
     *
     * @param idx The index of the element.
     * @return Reference to the element at the specified index.
     */
    [[nodiscard]] T &operator[](const std::size_t idx) noexcept {
        return *reinterpret_cast<T *>(&data + aligned_size * idx);
    }

    /**
     * @brief Operator to access elements by index (const version).
     *
     * @param idx The index of the element.
     * @return Const reference to the element at the specified index.
     */
    [[nodiscard]] const T &operator[](const std::size_t idx) const noexcept {
        return *reinterpret_cast<const T *>(&data + aligned_size * idx);
    }

    /**
     * @brief Adds a new element to the end of the Stack_Vector.
     *
     * @tparam Param The parameter types for constructing the new element.
     * @param param The arguments to forward to the constructor of the new
     * element.
     * @return Reference to the newly added element.
     */
    template <typename... Param>
    T &emplace_back(Param &&...param) {
        auto *p = new (&(*this)[m_size++]) T(std::forward<Param>(param)...);
        return *p;
    };

    /**
     * @brief Gets the number of elements stored in the Stack_Vector.
     *
     * @return The number of elements.
     */
    auto size() const noexcept { return m_size; };

    /**
     * @brief Gets the maximum capacity of the Stack_Vector.
     *
     * @return The maximum capacity.
     */
    auto capacity() const noexcept { return MaxSize; };

    /**
     * @brief Removes the last element from the Stack_Vector.
     */
    void pop_back() noexcept(std::is_nothrow_destructible_v<T>) {
        (*this)[--m_size].~T();
    }

    /**
     * @brief Resizes the Stack_Vector to contain the specified number of
     * elements.
     *
     * @param new_size The new size of the Stack_Vector.
     */
    void resize(std::size_t new_size) { m_size = new_size; }

    /**
     * @brief Copy assignment operator.
     *
     * @param other The Stack_Vector to copy from.
     * @return Reference to the modified Stack_Vector.
     */
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

    /**
     * @brief Move assignment operator.
     *
     * @param other The Stack_Vector to move from.
     * @return Reference to the modified Stack_Vector.
     */
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

    /**
     * @brief Size of each aligned element in the Stack_Vector.
     */
    constexpr static auto aligned_size =
        sizeof(T) + (sizeof(T) & std::alignment_of_v<T>) > 0
            ? std::alignment_of_v<T>
            : 0;

    std::size_t m_size{
        0}; /**< Current number of elements in the Stack_Vector. */
    alignas(std::alignment_of_v<T>) char data
        [aligned_size * MaxSize]; /**< Storage for the elements. */
};

#endif