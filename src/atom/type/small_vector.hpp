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
#include <numeric>
#include <stdexcept>
#include <type_traits>

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

    SmallVector(SmallVector&& other) noexcept { move(std::move(other)); }

    ~SmallVector() {
        clear();
        deallocate();
    }

    SmallVector& operator=(const SmallVector& other) {
        if (this != &other) {
            assign(other.begin(), other.end());
        }
        return *this;
    }

    SmallVector& operator=(SmallVector&& other) noexcept {
        if (this != &other) {
            clear();
            deallocate();
            move(std::move(other));
        }
        return *this;
    }

    SmallVector& operator=(std::initializer_list<T> init) {
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

    reference at(size_type pos) {
        if (pos >= size()) {
            throw std::out_of_range("SmallVector::at");
        }
        return (*this)[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size()) {
            throw std::out_of_range("SmallVector::at");
        }
        return (*this)[pos];
    }

    reference operator[](size_type pos) { return *(begin() + pos); }

    const_reference operator[](size_type pos) const { return *(begin() + pos); }

    reference front() { return *begin(); }

    const_reference front() const { return *begin(); }

    reference back() { return *(end() - 1); }

    const_reference back() const { return *(end() - 1); }

    T* data() noexcept { return begin(); }

    const T* data() const noexcept { return begin(); }

    iterator begin() noexcept {
        return capacity() > N ? data_ : static_buffer_.data();
    }

    const_iterator begin() const noexcept {
        return capacity() > N ? data_ : static_buffer_.data();
    }

    const_iterator cbegin() const noexcept { return begin(); }

    iterator end() noexcept { return begin() + size(); }

    const_iterator end() const noexcept { return begin() + size(); }

    const_iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept { return rend(); }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    size_type size() const noexcept { return size_; }

    size_type max_size() const noexcept {
        return std::numeric_limits<difference_type>::max();
    }

    void reserve(size_type new_cap) {
        if (new_cap > capacity()) {
            T* new_data = allocate(new_cap);
            std::move(begin(), end(), new_data);
            deallocate();
            data_ = new_data;
            capacity_ = new_cap;
        }
    }

    size_type capacity() const noexcept {
        return capacity_ > N ? capacity_ : N;
    }

    void clear() noexcept { size_ = 0; }

    iterator insert(const_iterator pos, const T& value) {
        return insert(pos, 1, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const T& value) {
        size_type index = pos - begin();
        if (size() + count > capacity()) {
            size_type new_cap = std::max(size() + count, capacity() * 2);
            T* new_data = allocate(new_cap);
            std::move(begin(), begin() + index, new_data);
            std::fill_n(new_data + index, count, value);
            std::move(begin() + index, end(), new_data + index + count);
            deallocate();
            data_ = new_data;
            capacity_ = new_cap;
        } else {
            std::move_backward(begin() + index, end(), end() + count);
            std::fill_n(begin() + index, count, value);
        }
        size_ += count;
        return begin() + index;
    }

    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        size_type index = pos - begin();
        size_type count = std::distance(first, last);
        if (size() + count > capacity()) {
            size_type new_cap = std::max(size() + count, capacity() * 2);
            T* new_data = allocate(new_cap);
            std::move(begin(), begin() + index, new_data);
            std::copy(first, last, new_data + index);
            std::move(begin() + index, end(), new_data + index + count);
            deallocate();
            data_ = new_data;
            capacity_ = new_cap;
        } else {
            std::move_backward(begin() + index, end(), end() + count);
            std::copy(first, last, begin() + index);
        }
        size_ += count;
        return begin() + index;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> init) {
        return insert(pos, init.begin(), init.end());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        size_type index = pos - begin();
        if (size() == capacity()) {
            size_type new_cap = capacity() == 0 ? 1 : capacity() * 2;
            T* new_data = allocate(new_cap);
            std::move(begin(), begin() + index, new_data);
            new (new_data + index) T(std::forward<Args>(args)...);
            std::move(begin() + index, end(), new_data + index + 1);
            deallocate();
            data_ = new_data;
            capacity_ = new_cap;
        } else {
            std::move_backward(begin() + index, end(), end() + 1);
            *(begin() + index) = T(std::forward<Args>(args)...);
        }
        ++size_;
        return begin() + index;
    }

    iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

    iterator erase(const_iterator first, const_iterator last) {
        size_type index = first - begin();
        size_type count = last - first;
        std::move(begin() + index + count, end(), begin() + index);
        size_ -= count;
        return begin() + index;
    }

    void push_back(const T& value) { emplace_back(value); }

    void push_back(T&& value) { emplace_back(std::move(value)); }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (size() == capacity()) {
            reserve(capacity() == 0 ? 1 : capacity() * 2);
        }
        new (end()) T(std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    void pop_back() { --size_; }

    void resize(size_type count, const T& value = T()) {
        if (count < size()) {
            erase(begin() + count, end());
        } else if (count > size()) {
            insert(end(), count - size(), value);
        }
    }

    void swap(SmallVector& other) noexcept {
        using std::swap;
        if (capacity() > N && other.capacity() > N) {
            swap(data_, other.data_);
        } else if (capacity() > N) {
            T* temp_data = data_;
            data_ = other.allocate(capacity());
            std::move(other.begin(), other.end(), data_);
            other.deallocate();
            other.data_ = temp_data;
        } else if (other.capacity() > N) {
            T* temp_data = other.data_;
            other.data_ = allocate(other.capacity());
            std::move(begin(), end(), other.data_);
            deallocate();
            data_ = temp_data;
        } else {
            swap(static_buffer_, other.static_buffer_);
        }
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

private:
    T* allocate(size_type n) {
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
bool operator==(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, std::size_t N>
bool operator!=(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return !(lhs == rhs);
}

template <typename T, std::size_t N>
bool operator<(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename T, std::size_t N>
bool operator<=(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return !(rhs < lhs);
}

template <typename T, std::size_t N>
bool operator>(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return rhs < lhs;
}

template <typename T, std::size_t N>
bool operator>=(const SmallVector<T, N>& lhs, const SmallVector<T, N>& rhs) {
    return !(lhs < rhs);
}

template <typename T, std::size_t N>
void swap(SmallVector<T, N>& lhs, SmallVector<T, N>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif
