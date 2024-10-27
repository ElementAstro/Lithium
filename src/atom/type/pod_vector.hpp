#ifndef ATOM_TYPE_POD_VECTOR_HPP
#define ATOM_TYPE_POD_VECTOR_HPP

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

#include "atom/atom/macro.hpp"

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
    static constexpr int SIZE_T = sizeof(T);
    static constexpr int N = 64 / SIZE_T;

    static_assert(N >= 4, "Element size too large");

private:
    int size_ = 0;
    int capacity_ = N;
    std::allocator<T> allocator_;
    T* data_ = allocator_.allocate(capacity_);

public:
    using size_type = int;

    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(pointer ptr) : ptr_(ptr) {}

        auto operator*() const -> reference { return *ptr_; }
        auto operator->() -> pointer { return ptr_; }

        auto operator++() -> iterator& {
            ++ptr_;
            return *this;
        }

        auto operator++(int) -> iterator {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator--() -> iterator& {
            --ptr_;
            return *this;
        }

        auto operator--(int) -> iterator {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        auto operator+(difference_type offset) const -> iterator {
            return iterator(ptr_ + offset);
        }

        auto operator-(difference_type offset) const -> iterator {
            return iterator(ptr_ - offset);
        }

        auto operator+=(difference_type offset) -> iterator& {
            ptr_ += offset;
            return *this;
        }

        auto operator-=(difference_type offset) -> iterator& {
            ptr_ -= offset;
            return *this;
        }

        auto operator-(const iterator& other) const -> difference_type {
            return ptr_ - other.ptr_;
        }

        auto operator==(const iterator& other) const -> bool {
            return ptr_ == other.ptr_;
        }

        auto operator!=(const iterator& other) const -> bool {
            return ptr_ != other.ptr_;
        }

        auto operator<(const iterator& other) const -> bool {
            return ptr_ < other.ptr_;
        }

        auto operator>(const iterator& other) const -> bool {
            return ptr_ > other.ptr_;
        }

        auto operator<=(const iterator& other) const -> bool {
            return ptr_ <= other.ptr_;
        }

        auto operator>=(const iterator& other) const -> bool {
            return ptr_ >= other.ptr_;
        }

    private:
        pointer ptr_;
    };

    class const_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator(pointer ptr) : ptr_(ptr) {}

        auto operator*() const -> reference { return *ptr_; }
        auto operator->() const -> pointer { return ptr_; }

        auto operator++() -> const_iterator& {
            ++ptr_;
            return *this;
        }

        auto operator++(int) -> const_iterator {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator--() -> const_iterator& {
            --ptr_;
            return *this;
        }

        auto operator--(int) -> const_iterator {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        auto operator+(difference_type offset) const -> const_iterator {
            return const_iterator(ptr_ + offset);
        }

        auto operator-(difference_type offset) const -> const_iterator {
            return const_iterator(ptr_ - offset);
        }

        auto operator+=(difference_type offset) -> const_iterator& {
            ptr_ += offset;
            return *this;
        }

        auto operator-=(difference_type offset) -> const_iterator& {
            ptr_ -= offset;
            return *this;
        }

        auto operator-(const const_iterator& other) const -> difference_type {
            return ptr_ - other.ptr_;
        }

        auto operator==(const const_iterator& other) const -> bool {
            return ptr_ == other.ptr_;
        }

        auto operator!=(const const_iterator& other) const -> bool {
            return ptr_ != other.ptr_;
        }

        auto operator<(const const_iterator& other) const -> bool {
            return ptr_ < other.ptr_;
        }

        auto operator>(const const_iterator& other) const -> bool {
            return ptr_ > other.ptr_;
        }

        auto operator<=(const const_iterator& other) const -> bool {
            return ptr_ <= other.ptr_;
        }

        auto operator>=(const const_iterator& other) const -> bool {
            return ptr_ >= other.ptr_;
        }

    private:
        pointer ptr_;
    };

    constexpr PodVector() noexcept = default;

    constexpr PodVector(std::initializer_list<T> il)
        : size_(static_cast<int>(il.size())),
          capacity_(std::max(N, size_)),
          data_(allocator_.allocate(capacity_)) {
        std::ranges::copy(il, data_);
    }

    explicit constexpr PodVector(int size_)
        : size_(size_),
          capacity_(std::max(N, size_)),
          data_(allocator_.allocate(capacity_)) {}

    PodVector(const PodVector& other)
        : size_(other.size_),
          capacity_(other.capacity_),
          data_(allocator_.allocate(capacity_)) {
        std::memcpy(data_, other.data_, SIZE_T * size_);
    }

    PodVector(PodVector&& other) noexcept
        : size_(other.size_),
          capacity_(other.capacity_),
          data_(std::exchange(other.data_, nullptr)) {}

    auto operator=(PodVector&& other) noexcept -> PodVector& {
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

    constexpr void reserve(int cap) {
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

    constexpr void popBack() noexcept { size_--; }

    constexpr auto popxBack() -> T { return std::move(data_[--size_]); }

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

    constexpr auto operator[](int index) -> T& { return data_[index]; }
    constexpr auto operator[](int index) const -> const T& {
        return data_[index];
    }

    constexpr auto begin() noexcept -> iterator { return iterator(data_); }
    constexpr auto end() noexcept -> iterator {
        return iterator(data_ + size_);
    }
    constexpr auto begin() const noexcept -> const_iterator {
        return const_iterator(data_);
    }
    constexpr auto end() const noexcept -> const_iterator {
        return const_iterator(data_ + size_);
    }
    constexpr auto back() -> T& { return data_[size_ - 1]; }
    constexpr auto back() const -> const T& { return data_[size_ - 1]; }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return size_ == 0;
    }
    [[nodiscard]] constexpr auto size() const noexcept -> int { return size_; }
    constexpr auto data() noexcept -> T* { return data_; }
    constexpr auto data() const noexcept -> const T* { return data_; }
    constexpr void clear() noexcept { size_ = 0; }

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

    constexpr void erase(int i) {
        std::ranges::copy(data_ + i + 1, data_ + size_, data_ + i);
        size_--;
    }

    constexpr void reverse() { std::ranges::reverse(data_, data_ + size_); }

    constexpr void resize(int size_) {
        if (size_ > capacity_) {
            reserve(size_);
        }
        this->size_ = size_;
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
            allocator_.deallocate(data_, capacity_);
        }
    }

    [[nodiscard]] constexpr auto capacity() const noexcept -> int {
        return capacity_;
    }
};

}  // namespace atom::type

#endif  // ATOM_TYPE_POD_VECTOR_HPP
