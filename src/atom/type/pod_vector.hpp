#ifndef ATOM_TYPE_POD_VECTOR_HPP
#define ATOM_TYPE_POD_VECTOR_HPP

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#include "atom/macro.hpp"

namespace atom::type {
template <typename T>
concept PodType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

template <typename T>
concept ValueType = requires(T t) {
    { std::is_copy_constructible_v<T> };
    { std::is_move_constructible_v<T> };
};

template <PodType T, int Growth = 2>
class PodVector {
    static ATOM_CONSTEXPR int SIZE_T = sizeof(T);
    static ATOM_CONSTEXPR int N = 64 / SIZE_T;

    static_assert(N >= 4, "Element size_ too large");

private:
    int size_ = 0;
    int capacity_ = N;
    std::allocator<T> allocator_;
    T* data_ = allocator_.allocate(capacity_);

public:
    using size_type = int;

    ATOM_CONSTEXPR PodVector() ATOM_NOEXCEPT = default;

    ATOM_CONSTEXPR PodVector(std::initializer_list<T> il)
        : size_(static_cast<int>(il.size())),
          capacity_(std::max(N, size_)),
          data_(allocator_.allocate(capacity_)) {
        std::ranges::copy(il, data_);
    }

    explicit ATOM_CONSTEXPR PodVector(int size_)
        : size_(size_),
          capacity_(std::max(N, size_)),
          data_(allocator_.allocate(capacity_)) {}

    PodVector(const PodVector& other)
        : size_(other.size_),
          capacity_(other.capacity_),
          data_(allocator_.allocate(capacity_)) {
        std::memcpy(data_, other.data_, SIZE_T * size_);
    }

    PodVector(PodVector&& other) ATOM_NOEXCEPT
        : size_(other.size_),
          capacity_(other.capacity_),
          data_(std::exchange(other.data_, nullptr)) {}

    auto operator=(PodVector&& other) ATOM_NOEXCEPT -> PodVector& {
        if (this != &other) {
            if (data_ != nullptr) {
                allocator_.deallocate(data_, capacity_);
            }
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = std::exchange(other.data_, nullptr);
        }
        return *this;
    }

    auto operator=(const PodVector& other) -> PodVector& = delete;

    template <typename ValueT>
    void pushBack(ValueT&& t) {
        if (size_ == capacity_) [[unlikely]] {
            reserve(capacity_ * Growth);
        }
        data_[size_++] = std::forward<ValueT>(t);
    }

    template <typename... Args>
    void emplaceBack(Args&&... args) {
        if (size_ == capacity_) [[unlikely]] {
            reserve(capacity_ * Growth);
        }
        new (&data_[size_++]) T(std::forward<Args>(args)...);
    }

    ATOM_CONSTEXPR void reserve(int cap) {
        if (cap <= capacity_) [[likely]] {
            return;
        }
        T* newData = allocator_.allocate(cap);
        if (data_ != nullptr) {
            std::memcpy(newData, data_, SIZE_T * size_);
            allocator_.deallocate(data_, capacity_);
        }
        data_ = newData;
        capacity_ = cap;
    }

    ATOM_CONSTEXPR void popBack() ATOM_NOEXCEPT { size_--; }

    ATOM_CONSTEXPR auto popxBack() -> T { return std::move(data_[--size_]); }

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

    ATOM_CONSTEXPR auto operator[](int index) -> T& { return data_[index]; }
    ATOM_CONSTEXPR auto operator[](int index) const -> const T& {
        return data_[index];
    }

    ATOM_CONSTEXPR auto begin() ATOM_NOEXCEPT -> T* { return data_; }
    ATOM_CONSTEXPR auto end() ATOM_NOEXCEPT -> T* { return data_ + size_; }
    ATOM_CONSTEXPR auto begin() const ATOM_NOEXCEPT -> const T* { return data_; }
    ATOM_CONSTEXPR auto end() const ATOM_NOEXCEPT -> const T* { return data_ + size_; }
    ATOM_CONSTEXPR auto back() -> T& { return data_[size_ - 1]; }
    ATOM_CONSTEXPR auto back() const -> const T& { return data_[size_ - 1]; }

    ATOM_NODISCARD ATOM_CONSTEXPR auto empty() const ATOM_NOEXCEPT -> bool {
        return size_ == 0;
    }
    ATOM_NODISCARD ATOM_CONSTEXPR auto size() const ATOM_NOEXCEPT -> int { return size_; }
    ATOM_CONSTEXPR auto data() ATOM_NOEXCEPT -> T* { return data_; }
    ATOM_CONSTEXPR auto data() const ATOM_NOEXCEPT -> const T* { return data_; }
    ATOM_CONSTEXPR void clear() ATOM_NOEXCEPT { size_ = 0; }

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

    ATOM_CONSTEXPR void erase(int i) {
        std::ranges::copy(data_ + i + 1, data_ + size_, data_ + i);
        size_--;
    }

    ATOM_CONSTEXPR void reverse() { std::ranges::reverse(data_, data_ + size_); }

    ATOM_CONSTEXPR void resize(int size_) {
        if (size_ > capacity_) {
            reserve(size_);
        }
        this->size_ = size_;
    }

    auto detach() ATOM_NOEXCEPT -> std::pair<T*, int> {
        T* p = data_;
        int size = size_;
        data_ = nullptr;
        size_ = 0;
        return {p, size};
    }

    ~PodVector() {
        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto capacity() const ATOM_NOEXCEPT -> int {
        return capacity_;
    }
};

}  // namespace atom::type

#endif
