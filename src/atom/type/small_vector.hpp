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

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include "error/exception.hpp"
#include "macro.hpp"

template <typename T, std::size_t N>
class SmallVector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    SmallVector() = default;

    template <typename InputIt,
              typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    SmallVector(InputIt first, InputIt last) {
        assign(first, last);
    }

    explicit SmallVector(size_type count, const T& value = T()) {
        assign(count, value);
    }

    template <typename InputIt>
    SmallVector(InputIt first, InputIt last) {
        assign(first, last);
    }

    SmallVector(std::initializer_list<T> init) { assign(init); }

    SmallVector(const SmallVector& other) {
        assign(other.begin(), other.end());
    }

    SmallVector(SmallVector&& other) ATOM_NOEXCEPT { move(std::move(other)); }

    ~SmallVector() {
        clear();
        deallocate();
    }

    auto operator=(const SmallVector& other) -> SmallVector& {
        if (this != &other) {
            assign(other.begin(), other.end());
        }
        return *this;
    }

    auto operator=(SmallVector&& other) ATOM_NOEXCEPT->SmallVector& {
        if (this != &other) {
            clear();
            deallocate();
            move(std::move(other));
        }
        return *this;
    }

    auto operator=(std::initializer_list<T> init) -> SmallVector& {
        assign(init);
        return *this;
    }

    void assign(size_type count, const T& value) {
        clear();
        if (count > capacity()) {
            deallocate();
            allocate(count);
        }
        std::fill_n(begin(), count, value);
        size_ = count;
    }

    template <typename InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        size_type count = std::distance(first, last);
        if (count > capacity()) {
            deallocate();
            allocate(count);
        }
        std::copy(first, last, begin());
        size_ = count;
    }

    void assign(std::initializer_list<T> init) {
        assign(init.begin(), init.end());
    }

    auto at(size_type pos) -> reference {
        if (pos >= size()) {
            THROW_OUT_OF_RANGE("SmallVector::at");
        }
        return (*this)[pos];
    }

    auto at(size_type pos) const -> const_reference {
        if (pos >= size()) {
            THROW_OUT_OF_RANGE("SmallVector::at");
        }
        return (*this)[pos];
    }

    auto operator[](size_type pos) -> reference { return *(begin() + pos); }

    auto operator[](size_type pos) const -> const_reference {
        return *(begin() + pos);
    }

    auto front() -> reference { return *begin(); }

    auto front() const -> const_reference { return *begin(); }

    auto back() -> reference { return *(end() - 1); }

    auto back() const -> const_reference { return *(end() - 1); }

    auto data() ATOM_NOEXCEPT -> T* { return begin(); }

    auto data() const ATOM_NOEXCEPT -> const T* { return begin(); }

    auto begin() ATOM_NOEXCEPT -> iterator {
        return capacity() > N ? data_ : static_buffer_.data();
    }

    auto begin() const ATOM_NOEXCEPT -> const_iterator {
        return capacity() > N ? data_ : static_buffer_.data();
    }

    auto cbegin() const ATOM_NOEXCEPT -> const_iterator { return begin(); }

    auto end() ATOM_NOEXCEPT -> iterator { return begin() + size(); }

    auto end() const ATOM_NOEXCEPT -> const_iterator {
        return begin() + size();
    }

    auto cend() const ATOM_NOEXCEPT -> const_iterator { return end(); }

    auto rbegin() ATOM_NOEXCEPT -> reverse_iterator {
        return reverse_iterator(end());
    }

    auto rbegin() const ATOM_NOEXCEPT -> const_reverse_iterator {
        return const_reverse_iterator(end());
    }

    auto crbegin() const ATOM_NOEXCEPT -> const_reverse_iterator {
        return rbegin();
    }

    auto rend() ATOM_NOEXCEPT -> reverse_iterator {
        return reverse_iterator(begin());
    }

    auto rend() const ATOM_NOEXCEPT -> const_reverse_iterator {
        return const_reverse_iterator(begin());
    }

    auto crend() const ATOM_NOEXCEPT -> const_reverse_iterator {
        return rend();
    }

    ATOM_NODISCARD auto empty() const ATOM_NOEXCEPT -> bool {
        return size() == 0;
    }

    ATOM_NODISCARD auto size() const ATOM_NOEXCEPT -> size_type {
        return size_;
    }

    ATOM_NODISCARD auto maxSize() const ATOM_NOEXCEPT -> size_type {
        return std::numeric_limits<difference_type>::max();
    }

    void reserve(size_type new_cap) {
        if (new_cap > capacity()) {
            T* newData = allocate(new_cap);
            std::move(begin(), end(), newData);
            deallocate();
            data_ = newData;
            capacity_ = new_cap;
        }
    }

    ATOM_NODISCARD auto capacity() const ATOM_NOEXCEPT -> size_type {
        return capacity_ > N ? capacity_ : N;
    }

    void clear() ATOM_NOEXCEPT { size_ = 0; }

    auto insert(const_iterator pos, const T& value) -> iterator {
        return insert(pos, 1, value);
    }

    auto insert(const_iterator pos, T&& value) -> iterator {
        return emplace(pos, std::move(value));
    }

    auto insert(const_iterator pos, size_type count,
                const T& value) -> iterator {
        size_type index = pos - begin();
        if (size() + count > capacity()) {
            size_type newCap = std::max(size() + count, capacity() * 2);
            T* newData = allocate(newCap);
            std::move(begin(), begin() + index, newData);
            std::fill_n(newData + index, count, value);
            std::move(begin() + index, end(), newData + index + count);
            deallocate();
            data_ = newData;
            capacity_ = newCap;
        } else {
            std::move_backward(begin() + index, end(), end() + count);
            std::fill_n(begin() + index, count, value);
        }
        size_ += count;
        return begin() + index;
    }

    template <typename InputIt>
    auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator {
        size_type index = pos - begin();
        size_type count = std::distance(first, last);
        if (size() + count > capacity()) {
            size_type newCap = std::max(size() + count, capacity() * 2);
            T* newData = allocate(newCap);
            std::move(begin(), begin() + index, newData);
            std::copy(first, last, newData + index);
            std::move(begin() + index, end(), newData + index + count);
            deallocate();
            data_ = newData;
            capacity_ = newCap;
        } else {
            std::move_backward(begin() + index, end(), end() + count);
            std::copy(first, last, begin() + index);
        }
        size_ += count;
        return begin() + index;
    }

    auto insert(const_iterator pos, std::initializer_list<T> init) -> iterator {
        return insert(pos, init.begin(), init.end());
    }

    template <typename... Args>
    auto emplace(const_iterator pos, Args&&... args) -> iterator {
        size_type index = pos - begin();
        if (size() == capacity()) {
            size_type newCap = capacity() == 0 ? 1 : capacity() * 2;
            T* newData = allocate(newCap);
            std::move(begin(), begin() + index, newData);
            new (newData + index) T(std::forward<Args>(args)...);
            std::move(begin() + index, end(), newData + index + 1);
            deallocate();
            data_ = newData;
            capacity_ = newCap;
        } else {
            std::move_backward(begin() + index, end(), end() + 1);
            *(begin() + index) = T(std::forward<Args>(args)...);
        }
        ++size_;
        return begin() + index;
    }

    auto erase(const_iterator pos) -> iterator { return erase(pos, pos + 1); }

    auto erase(const_iterator first, const_iterator last) -> iterator {
        size_type index = first - begin();
        size_type count = last - first;
        std::move(begin() + index + count, end(), begin() + index);
        size_ -= count;
        return begin() + index;
    }

    void pushBack(const T& value) { emplace_back(value); }

    void pushBack(T&& value) { emplace_back(std::move(value)); }

    template <typename... Args>
    auto emplaceBack(Args&&... args) -> reference {
        if (size() == capacity()) {
            reserve(capacity() == 0 ? 1 : capacity() * 2);
        }
        new (end()) T(std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    void popBack() { --size_; }

    void resize(size_type count, const T& value = T()) {
        if (count < size()) {
            erase(begin() + count, end());
        } else if (count > size()) {
            insert(end(), count - size(), value);
        }
    }

    void swap(SmallVector& other) ATOM_NOEXCEPT {
        using std::swap;
        if (capacity() > N && other.capacity() > N) {
            swap(data_, other.data_);
        } else if (capacity() > N) {
            T* tempData = data_;
            data_ = other.allocate(capacity());
            std::move(other.begin(), other.end(), data_);
            other.deallocate();
            other.data_ = tempData;
        } else if (other.capacity() > N) {
            T* tempData = other.data_;
            other.data_ = allocate(other.capacity());
            std::move(begin(), end(), other.data_);
            deallocate();
            data_ = tempData;
        } else {
            swap(static_buffer_, other.static_buffer_);
        }
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

private:
    auto allocate(size_type n) -> T* {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate() {
        if (capacity() > N) {
            ::operator delete(data_);
        }
    }

    void move(SmallVector&& other) {
        if (other.capacity() > N) {
            data_ = other.data_;
            other.data_ = nullptr;
        } else {
            std::move(other.begin(), other.end(), begin());
        }
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.size_ = 0;
        other.capacity_ = N;
    }

    size_type size_ = 0;
    size_type capacity_ = N;
    std::array<T, N> static_buffer_;
    T* data_ = nullptr;
};

template <typename T, std::size_t N>
auto operator==(const SmallVector<T, N>& lhs,
                const SmallVector<T, N>& rhs) -> bool {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, std::size_t N>
auto operator!=(const SmallVector<T, N>& lhs,
                const SmallVector<T, N>& rhs) -> bool {
    return !(lhs == rhs);
}

template <typename T, std::size_t N>
auto operator<(const SmallVector<T, N>& lhs,
               const SmallVector<T, N>& rhs) -> bool {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename T, std::size_t N>
auto operator<=(const SmallVector<T, N>& lhs,
                const SmallVector<T, N>& rhs) -> bool {
    return !(rhs < lhs);
}

template <typename T, std::size_t N>
auto operator>(const SmallVector<T, N>& lhs,
               const SmallVector<T, N>& rhs) -> bool {
    return rhs < lhs;
}

template <typename T, std::size_t N>
auto operator>=(const SmallVector<T, N>& lhs,
                const SmallVector<T, N>& rhs) -> bool {
    return !(lhs < rhs);
}

template <typename T, std::size_t N>
void swap(SmallVector<T, N>& lhs, SmallVector<T, N>& rhs) ATOM_NOEXCEPT {
    lhs.swap(rhs);
}

#endif
