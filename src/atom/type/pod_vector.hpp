#ifndef ATOM_TYPE_POD_VECTOR_HPP
#define ATOM_TYPE_POD_VECTOR_HPP

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <vector>

#include "atom/macro.hpp"

namespace atom::type {
template <typename T>
concept PodType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

ATOM_INLINE auto pool64Alloc(std::size_t size) -> void* {
    return std::malloc(size);
}

ATOM_INLINE void pool64Dealloc(void* ptr) { std::free(ptr); }

template <PodType T, int Growth = 2>
struct PodVector {
    static constexpr int SIZE_T = sizeof(T);
    static constexpr int N = 64 / SIZE_T;

    static_assert(N >= 4, "Element size_ too large");

private:
    int size_;
    int capacity_;
    T* data_;

public:
    using size_type = int;

    PodVector() : size_(0), capacity_(N) {
        data_ = static_cast<T*>(
            pool64Alloc(static_cast<std::size_t>(capacity_ * SIZE_T)));
    }

    PodVector(std::initializer_list<T> il)
        : size_(il.size()), capacity_(std::max(N, size_)) {
        data_ = static_cast<T*>(
            pool64Alloc(static_cast<std::size_t>(capacity_ * SIZE_T)));
        std::copy(il.begin(), il.end(), data_);
    }

    explicit PodVector(int size_)
        : size_(size_), capacity_(std::max(N, size_)) {
        data_ = static_cast<T*>(
            pool64Alloc(static_cast<std::size_t>(capacity_ * SIZE_T)));
    }

    PodVector(const PodVector& other)
        : size_(other.size_), capacity_(other.capacity_) {
        data_ = static_cast<T*>(
            pool64Alloc(static_cast<std::size_t>(capacity_ * SIZE_T)));
        std::memcpy(data_, other.data_, SIZE_T * size_);
    }

    PodVector(PodVector&& other) noexcept
        : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
        other.data_ = nullptr;
    }

    auto operator=(PodVector&& other) noexcept -> PodVector& {
        if (this != &other) {
            if (data_ != nullptr) {
                pool64Dealloc(data_);
            }
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.data_ = nullptr;
        }
        return *this;
    }

    auto operator=(const PodVector& other) -> PodVector& = delete;

    template <typename ValueT>
    void pushBack(ValueT&& t) {
        if (size_ == capacity_) {
            reserve(capacity_ * Growth);
        }
        data_[size_++] = std::forward<ValueT>(t);
    }

    template <typename... Args>
    void emplaceBack(Args&&... args) {
        if (size_ == capacity_) {
            reserve(capacity_ * Growth);
        }
        new (&data_[size_++]) T(std::forward<Args>(args)...);
    }

    void reserve(int cap) {
        if (cap <= capacity_) {
            return;
        }
        capacity_ = cap;
        T* oldData = data_;
        data_ = static_cast<T*>(
            pool64Alloc(static_cast<std::size_t>(capacity_ * SIZE_T)));
        if (oldData != nullptr) {
            std::memcpy(data_, oldData, SIZE_T * size_);
            pool64Dealloc(oldData);
        }
    }

    void popBack() { size_--; }
    auto popxBack() -> T {
        T t = std::move(data_[size_ - 1]);
        size_--;
        return t;
    }

    void extend(const PodVector& other) {
        for (const auto& elem : other) {
            pushBack(elem);
        }
    }

    void extend(const T* begin, const T* end) {
        for (auto it = begin; it != end; ++it) {
            pushBack(*it);
        }
    }

    auto operator[](int index) -> T& { return data_[index]; }
    auto operator[](int index) const -> const T& { return data_[index]; }

    auto begin() -> T* { return data_; }
    auto end() -> T* { return data_ + size_; }
    auto begin() const -> const T* { return data_; }
    auto end() const -> const T* { return data_ + size_; }
    auto back() -> T& { return data_[size_ - 1]; }
    auto back() const -> const T& { return data_[size_ - 1]; }

    [[nodiscard]] auto empty() const -> bool { return size_ == 0; }
    [[nodiscard]] auto size() const -> int { return size_; }
    auto data() -> T* { return data_; }
    auto data() const -> const T* { return data_; }
    void clear() { size_ = 0; }

    template <typename ValueT>
    void insert(int i, ValueT&& val) {
        if (size_ == capacity_) {
            reserve(capacity_ * Growth);
        }
        for (int j = size_; j > i; j--) {
            data_[j] = data_[j - 1];
        }
        data_[i] = std::forward<ValueT>(val);
        size_++;
    }

    void erase(int i) {
        for (int j = i; j < size_ - 1; j++) {
            data_[j] = data_[j + 1];
        }
        size_--;
    }

    void reverse() { std::reverse(data_, data_ + size_); }

    void resize(int size_) {
        if (size_ > capacity_) {
            reserve(size_);
        }
        size_ = size_;
    }

    auto detach() noexcept -> std::pair<T*, int> {
        T* p = data_;
        int size = size_;
        data_ = nullptr;
        size_ = 0;
        return {p, size};
    }

    ~PodVector() {
        if (data_ != nullptr) {
            pool64Dealloc(data_);
        }
    }

    [[nodiscard]] auto capacity() const -> size_t { return capacity_; }
};

template <typename T, typename Container = std::vector<T>>
class Stack {
    Container vec_;

public:
    void push(const T& t) { vec_.push_back(t); }
    void push(T&& t) { vec_.push_back(std::move(t)); }
    template <typename... Args>
    void emplace(Args&&... args) {
        vec_.emplace_back(std::forward<Args>(args)...);
    }
    void pop() { vec_.pop_back(); }
    void clear() { vec_.clear(); }
    [[nodiscard]] auto empty() const -> bool { return vec_.empty(); }
    auto size() const -> typename Container::size_type { return vec_.size(); }
    auto top() -> T& { return vec_.back(); }
    auto top() const -> const T& { return vec_.back(); }
    auto popx() -> T {
        T t = std::move(vec_.back());
        vec_.pop_back();
        return t;
    }
    void reserve(int n) { vec_.reserve(n); }
    auto container() -> Container& { return vec_; }
    auto container() const -> const Container& { return vec_; }
};

}  // namespace atom::type

#endif
