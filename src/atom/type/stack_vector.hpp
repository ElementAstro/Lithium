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
struct StackVector {
    /**
     * @brief Default constructor.
     */
    StackVector() noexcept = default;

    /**
     * @brief Copy constructor.
     *
     * @param other The Stack_Vector to copy from.
     */
    StackVector(const StackVector &other) {
        mSize = other.mSize;
        for (std::size_t i = 0; i < mSize; ++i) {
            new (&(*this)[i]) T(other[i]);
        }
    }

    /**
     * @brief Move constructor.
     *
     * @param other The Stack_Vector to move from.
     */
    StackVector(StackVector &&other) noexcept
        : data{other.data}, mSize{other.mSize} {
        other.mSize = 0;
    }

    /**
     * @brief Destructor.
     */
    ~StackVector() noexcept(std::is_nothrow_destructible_v<T>) {
        for (std::size_t pos = 0; pos < mSize; ++pos) {
            (*this)[pos].~T();
        }
    }

    /**
     * @brief Operator to access elements by index.
     *
     * @param idx The index of the element.
     * @return Reference to the element at the specified index.
     */
    [[nodiscard]] auto operator[](const std::size_t IDX) noexcept -> T & {
        return *reinterpret_cast<T *>(&data + ALIGNED_SIZE * IDX);
    }

    /**
     * @brief Operator to access elements by index (const version).
     *
     * @param idx The index of the element.
     * @return Const reference to the element at the specified index.
     */
    [[nodiscard]] auto operator[](const std::size_t IDX) const noexcept -> const T & {
        return *reinterpret_cast<const T *>(&data + ALIGNED_SIZE * IDX);
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
    auto emplaceBack(Param &&...param) -> T & {
        auto *p = new (&(*this)[mSize++]) T(std::forward<Param>(param)...);
        return *p;
    };

    /**
     * @brief Gets the number of elements stored in the Stack_Vector.
     *
     * @return The number of elements.
     */
    auto size() const noexcept { return mSize; };

    /**
     * @brief Gets the maximum capacity of the Stack_Vector.
     *
     * @return The maximum capacity.
     */
    auto capacity() const noexcept { return MaxSize; };

    /**
     * @brief Removes the last element from the Stack_Vector.
     */
    void popBack() noexcept(std::is_nothrow_destructible_v<T>) {
        (*this)[--mSize].~T();
    }

    /**
     * @brief Resizes the Stack_Vector to contain the specified number of
     * elements.
     *
     * @param new_size The new size of the Stack_Vector.
     */
    void resize(std::size_t new_size) { mSize = new_size; }

    /**
     * @brief Copy assignment operator.
     *
     * @param other The Stack_Vector to copy from.
     * @return Reference to the modified Stack_Vector.
     */
    auto operator=(const StackVector &other) -> StackVector & {
        if (this != &other) {
            for (std::size_t i = 0; i < mSize; ++i) {
                (*this)[i].~T();
            }
            mSize = other.mSize;
            for (std::size_t i = 0; i < mSize; ++i) {
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
    auto operator=(StackVector &&other) noexcept -> StackVector & {
        if (this != &other) {
            for (std::size_t i = 0; i < mSize; ++i) {
                (*this)[i].~T();
            }
            std::swap(data, other.data);
            std::swap(mSize, other.mSize);
        }
        return *this;
    }

    /**
     * @brief Size of each aligned element in the Stack_Vector.
     */
    constexpr static auto ALIGNED_SIZE =
        sizeof(T) + (sizeof(T) & std::alignment_of_v<T>) > 0
            ? std::alignment_of_v<T>
            : 0;

    std::size_t mSize{
        0}; /**< Current number of elements in the Stack_Vector. */
    alignas(std::alignment_of_v<T>) char data
        [ALIGNED_SIZE * MaxSize]{}; /**< Storage for the elements. */
};

#endif
