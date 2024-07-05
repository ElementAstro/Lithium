#ifndef ATOM_ALGORITHM_RING_HPP
#define ATOM_ALGORITHM_RING_HPP

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t size) : buffer_(size), max_size_(size) {}

    auto push(const T& item) -> bool {
        if (full()) {
            return false;  // Buffer is full, cannot push new item
        }
        buffer_[head_] = item;
        head_ = (head_ + 1) % max_size_;
        ++count_;
        return true;
    }

    auto pop() -> std::optional<T> {
        if (empty()) {
            return std::nullopt;  // Buffer is empty, nothing to pop
        }
        T item = buffer_[tail_];
        tail_ = (tail_ + 1) % max_size_;
        --count_;
        return item;
    }

    [[nodiscard]] auto full() const -> bool { return count_ == max_size_; }

    [[nodiscard]] auto empty() const -> bool { return count_ == 0; }

    [[nodiscard]] auto size() const -> size_t { return count_; }

    void clear() {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    auto front() const -> std::optional<T> {
        if (empty()) {
            return std::nullopt;
        }
        return buffer_[tail_];
    }

    auto back() const -> std::optional<T> {
        if (empty()) {
            return std::nullopt;
        }
        return buffer_[(head_ + max_size_ - 1) % max_size_];
    }

    auto contains(const T& item) const -> bool {
        return std::ranges::any_of(
            buffer_ | std::views::take(count_),
            [&item](const T& elem) { return elem == item; });
    }

    [[nodiscard]] auto view() const {
        auto firstPart = std::span(buffer_.data() + tail_,
                                   std::min(count_, max_size_ - tail_));
        auto secondPart = std::span(
            buffer_.data(),
            count_ > max_size_ - tail_ ? count_ - (max_size_ - tail_) : 0);

        std::vector<T> combined;
        combined.reserve(count_);
        std::ranges::copy(firstPart, std::back_inserter(combined));
        std::ranges::copy(secondPart, std::back_inserter(combined));

        return combined;
    }

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        Iterator(pointer buf, size_t max_size, size_t index, size_t count)
            : buf_(buf), max_size_(max_size), index_(index), count_(count) {}

        auto operator*() const -> reference { return buf_[index_]; }
        auto operator->() -> pointer { return &buf_[index_]; }

        auto operator++() -> Iterator& {
            ++pos_;
            if (pos_ < count_) {
                index_ = (index_ + 1) % max_size_;
            }
            return *this;
        }

        auto operator++(int) -> Iterator {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
            return a.pos_ == b.pos_;
        }

        friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
            return a.pos_ != b.pos_;
        }

    private:
        pointer buf_;
        size_t max_size_;
        size_t index_;
        size_t count_;
        size_t pos_ = 0;
    };

    auto begin() const -> Iterator {
        return Iterator(buffer_.data(), max_size_, tail_, count_);
    }

    auto end() const -> Iterator {
        return Iterator(buffer_.data(), max_size_, tail_, count_);
    }

private:
    std::vector<T> buffer_;
    size_t max_size_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};

#endif