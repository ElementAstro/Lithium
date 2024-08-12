#ifndef ATOM_ALGORITHM_RING_HPP
#define ATOM_ALGORITHM_RING_HPP

#include <algorithm>
#include <concepts>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

/**
 * @brief A circular buffer implementation.
 *
 * @tparam T The type of elements stored in the buffer.
 */
template <typename T>
class RingBuffer {
public:
    /**
     * @brief Construct a new RingBuffer object.
     *
     * @param size The maximum size of the buffer.
     */
    explicit RingBuffer(size_t size) : buffer_(size), max_size_(size) {}

    /**
     * @brief Push an item to the buffer.
     *
     * @param item The item to push.
     * @return true if the item was successfully pushed, false if the buffer was
     * full.
     */
    auto push(const T& item) -> bool {
        if (full()) {
            return false;
        }
        buffer_[head_] = item;
        head_ = (head_ + 1) % max_size_;
        ++count_;
        return true;
    }

    /**
     * @brief Push an item to the buffer, overwriting the oldest item if full.
     *
     * @param item The item to push.
     */
    void pushOverwrite(const T& item) {
        if (full()) {
            tail_ = (tail_ + 1) % max_size_;
        } else {
            ++count_;
        }
        buffer_[head_] = item;
        head_ = (head_ + 1) % max_size_;
    }

    /**
     * @brief Pop an item from the buffer.
     *
     * @return std::optional<T> The popped item, or std::nullopt if the buffer
     * was empty.
     */
    [[nodiscard]] auto pop() -> std::optional<T> {
        if (empty()) {
            return std::nullopt;
        }
        T item = buffer_[tail_];
        tail_ = (tail_ + 1) % max_size_;
        --count_;
        return item;
    }

    /**
     * @brief Check if the buffer is full.
     *
     * @return true if the buffer is full, false otherwise.
     */
    [[nodiscard]] auto full() const -> bool { return count_ == max_size_; }

    /**
     * @brief Check if the buffer is empty.
     *
     * @return true if the buffer is empty, false otherwise.
     */
    [[nodiscard]] auto empty() const -> bool { return count_ == 0; }

    /**
     * @brief Get the current number of items in the buffer.
     *
     * @return size_t The number of items in the buffer.
     */
    [[nodiscard]] auto size() const -> size_t { return count_; }

    /**
     * @brief Get the maximum size of the buffer.
     *
     * @return size_t The maximum size of the buffer.
     */
    [[nodiscard]] auto capacity() const -> size_t { return max_size_; }

    /**
     * @brief Clear all items from the buffer.
     */
    void clear() {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    /**
     * @brief Get the front item of the buffer without removing it.
     *
     * @return std::optional<T> The front item, or std::nullopt if the buffer is
     * empty.
     */
    [[nodiscard]] auto front() const -> std::optional<T> {
        if (empty()) {
            return std::nullopt;
        }
        return buffer_[tail_];
    }

    /**
     * @brief Get the back item of the buffer without removing it.
     *
     * @return std::optional<T> The back item, or std::nullopt if the buffer is
     * empty.
     */
    [[nodiscard]] auto back() const -> std::optional<T> {
        if (empty()) {
            return std::nullopt;
        }
        return buffer_[(head_ + max_size_ - 1) % max_size_];
    }

    /**
     * @brief Check if the buffer contains a specific item.
     *
     * @param item The item to search for.
     * @return true if the item is in the buffer, false otherwise.
     */
    [[nodiscard]] auto contains(const T& item) const -> bool {
        return std::ranges::any_of(
            buffer_ | std::views::take(count_),
            [&item](const T& elem) { return elem == item; });
    }

    /**
     * @brief Get a view of the buffer's contents.
     *
     * @return std::vector<T> A vector containing the buffer's contents in
     * order.
     */
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

    /**
     * @brief Iterator class for RingBuffer.
     */
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

    /**
     * @brief Get an iterator to the beginning of the buffer.
     *
     * @return Iterator
     */
    [[nodiscard]] auto begin() const -> Iterator {
        return Iterator(buffer_.data(), max_size_, tail_, count_);
    }

    /**
     * @brief Get an iterator to the end of the buffer.
     *
     * @return Iterator
     */
    [[nodiscard]] auto end() const -> Iterator {
        return Iterator(buffer_.data(), max_size_, tail_, count_);
    }

    /**
     * @brief Resize the buffer.
     *
     * @param new_size The new size of the buffer.
     */
    void resize(size_t new_size) {
        std::vector<T> newBuffer(new_size);
        size_t newCount = std::min(count_, new_size);

        for (size_t i = 0; i < newCount; ++i) {
            newBuffer[i] = buffer_[(tail_ + i) % max_size_];
        }

        buffer_ = std::move(newBuffer);
        max_size_ = new_size;
        head_ = newCount % new_size;
        tail_ = 0;
        count_ = newCount;
    }

    /**
     * @brief Get an element at a specific index in the buffer.
     *
     * @param index The index of the element to retrieve.
     * @return std::optional<T> The element at the specified index, or
     * std::nullopt if the index is out of bounds.
     */
    [[nodiscard]] auto at(size_t index) const -> std::optional<T> {
        if (index >= count_) {
            return std::nullopt;
        }
        return buffer_[(tail_ + index) % max_size_];
    }

    /**
     * @brief Apply a function to each element in the buffer.
     *
     * @param func The function to apply to each element.
     */
    template <std::invocable<T&> F>
    void forEach(F&& func) {
        for (size_t i = 0; i < count_; ++i) {
            func(buffer_[(tail_ + i) % max_size_]);
        }
    }

    /**
     * @brief Remove elements from the buffer that satisfy a predicate.
     *
     * @param pred The predicate function.
     */
    template <std::predicate<T> P>
    void removeIf(P&& pred) {
        size_t write = tail_;
        size_t read = tail_;
        size_t newCount = 0;

        for (size_t i = 0; i < count_; ++i) {
            if (!pred(buffer_[read])) {
                if (write != read) {
                    buffer_[write] = std::move(buffer_[read]);
                }
                write = (write + 1) % max_size_;
                ++newCount;
            }
            read = (read + 1) % max_size_;
        }

        count_ = newCount;
        head_ = write;
    }

    /**
     * @brief Rotate the buffer by a specified number of positions.
     *
     * @param n The number of positions to rotate. Positive values rotate left,
     * negative values rotate right.
     */
    void rotate(int n) {
        if (empty() || n == 0) {
            return;
        }

        n = n % static_cast<int>(count_);
        if (n < 0) {
            n += count_;
        }

        tail_ = (tail_ + n) % max_size_;
        head_ = (head_ + n) % max_size_;
    }

private:
    std::vector<T> buffer_;
    size_t max_size_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};

#endif  // ATOM_ALGORITHM_RING_HPP
