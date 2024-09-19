/*
 * small_list.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-17

Description: A Small List Implementation

**************************************************/

#ifndef ATOM_TYPE_SMALL_LIST_HPP
#define ATOM_TYPE_SMALL_LIST_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include "macro.hpp"

namespace atom::type {

/**
 * @brief A small doubly linked list implementation.
 *
 * @tparam T The type of elements stored in the list.
 */
template <typename T>
class SmallList {
    // Ensure that T is swappable and comparable
    static_assert(std::is_swappable_v<T>, "T must be swappable");
    static_assert(std::is_copy_constructible_v<T>,
                  "T must be copy constructible");

private:
    /**
     * @brief A node in the doubly linked list.
     */
    struct Node {
        T data;                      ///< The data stored in the node.
        std::unique_ptr<Node> next;  ///< Pointer to the next node.
        Node* prev;                  ///< Pointer to the previous node.

        /**
         * @brief Constructs a node with the given value.
         *
         * @param value The value to store in the node.
         */
        explicit Node(const T& value)
            : data(value), next(nullptr), prev(nullptr) {}
    };

    std::unique_ptr<Node> head_;  ///< Pointer to the head of the list.
    Node* tail_;                  ///< Pointer to the tail of the list.
    size_t list_size_;            ///< The number of elements in the list.

public:
    /**
     * @brief Default constructor. Constructs an empty list.
     */
    constexpr SmallList() noexcept
        : head_(nullptr), tail_(nullptr), list_size_(0) {}

    /**
     * @brief Constructs a list with elements from an initializer list.
     *
     * @param init_list The initializer list containing elements to add to the
     * list.
     */
    constexpr SmallList(std::initializer_list<T> init_list) : SmallList() {
        for (const auto& value : init_list) {
            pushBack(value);
        }
    }

    /**
     * @brief Destructor. Clears the list.
     */
    ~SmallList() noexcept { clear(); }

    /**
     * @brief Copy constructor. Constructs a list by copying elements from
     * another list.
     *
     * @param other The list to copy elements from.
     */
    SmallList(const SmallList& other) : SmallList() {
        for (const auto& value : other) {
            pushBack(value);
        }
    }

    /**
     * @brief Move constructor. Constructs a list by moving elements from
     * another list.
     *
     * @param other The list to move elements from.
     */
    SmallList(SmallList&& other) noexcept : SmallList() { swap(other); }

    /**
     * @brief Copy assignment operator. Copies elements from another list.
     *
     * @param other The list to copy elements from.
     * @return A reference to the assigned list.
     */
    SmallList& operator=(SmallList other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Adds an element to the end of the list.
     *
     * @param value The value to add.
     */
    constexpr void pushBack(const T& value) {
        auto newNode = std::make_unique<Node>(value);
        if (empty()) {
            head_ = std::move(newNode);
            tail_ = head_.get();
        } else {
            tail_->next = std::move(newNode);
            tail_->next->prev = tail_;
            tail_ = tail_->next.get();
        }
        ++list_size_;
    }

    /**
     * @brief Adds an element to the front of the list.
     *
     * @param value The value to add.
     */
    constexpr void pushFront(const T& value) {
        auto newNode = std::make_unique<Node>(value);
        if (empty()) {
            head_ = std::move(newNode);
            tail_ = head_.get();
        } else {
            newNode->next = std::move(head_);
            head_->prev = newNode.get();
            head_ = std::move(newNode);
        }
        ++list_size_;
    }

    /**
     * @brief Removes the last element from the list.
     */
    constexpr void popBack() noexcept {
        if (!empty()) {
            if (tail_ == head_.get()) {
                head_.reset();
                tail_ = nullptr;
            } else {
                tail_ = tail_->prev;
                tail_->next.reset();
            }
            --list_size_;
        }
    }

    /**
     * @brief Removes the first element from the list.
     */
    constexpr void popFront() noexcept {
        if (!empty()) {
            if (head_.get() == tail_) {
                head_.reset();
                tail_ = nullptr;
            } else {
                head_ = std::move(head_->next);
                head_->prev = nullptr;
            }
            --list_size_;
        }
    }

    /**
     * @brief Returns a reference to the first element in the list.
     *
     * @return A reference to the first element.
     */
    constexpr T& front() noexcept { return head_->data; }

    /**
     * @brief Returns a const reference to the first element in the list.
     *
     * @return A const reference to the first element.
     */
    constexpr const T& front() const noexcept { return head_->data; }

    /**
     * @brief Returns a reference to the last element in the list.
     *
     * @return A reference to the last element.
     */
    constexpr T& back() noexcept { return tail_->data; }

    /**
     * @brief Returns a const reference to the last element in the list.
     *
     * @return A const reference to the last element.
     */
    constexpr const T& back() const noexcept { return tail_->data; }

    /**
     * @brief Checks if the list is empty.
     *
     * @return True if the list is empty, false otherwise.
     */
    ATOM_NODISCARD constexpr bool empty() const noexcept {
        return list_size_ == 0;
    }

    /**
     * @brief Returns the number of elements in the list.
     *
     * @return The number of elements in the list.
     */
    ATOM_NODISCARD constexpr size_t size() const noexcept { return list_size_; }

    /**
     * @brief Clears the list, removing all elements.
     */
    constexpr void clear() noexcept {
        while (!empty()) {
            popFront();
        }
    }

    /**
     * @brief A bidirectional iterator for the list.
     */
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        /**
         * @brief Constructs an iterator pointing to the given node.
         *
         * @param ptr The node to point to.
         */
        explicit Iterator(Node* ptr = nullptr) noexcept : nodePtr(ptr) {}

        /**
         * @brief Advances the iterator to the next element.
         *
         * @return A reference to the iterator.
         */
        Iterator& operator++() noexcept {
            nodePtr = nodePtr->next.get();
            return *this;
        }

        /**
         * @brief Advances the iterator to the next element.
         *
         * @return A copy of the iterator before the increment.
         */
        Iterator operator++(int) noexcept {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * @brief Moves the iterator to the previous element.
         *
         * @return A reference to the iterator.
         */
        Iterator& operator--() noexcept {
            nodePtr = nodePtr->prev;
            return *this;
        }

        /**
         * @brief Moves the iterator to the previous element.
         *
         * @return A copy of the iterator before the decrement.
         */
        Iterator operator--(int) noexcept {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        /**
         * @brief Checks if two iterators are equal.
         *
         * @param other The iterator to compare with.
         * @return True if the iterators are equal, false otherwise.
         */
        bool operator==(const Iterator& other) const noexcept {
            return nodePtr == other.nodePtr;
        }

        /**
         * @brief Checks if two iterators are not equal.
         *
         * @param other The iterator to compare with.
         * @return True if the iterators are not equal, false otherwise.
         */
        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

        /**
         * @brief Dereferences the iterator to access the element.
         *
         * @return A reference to the element.
         */
        reference operator*() const noexcept { return nodePtr->data; }

        /**
         * @brief Dereferences the iterator to access the element.
         *
         * @return A pointer to the element.
         */
        pointer operator->() const noexcept { return &(nodePtr->data); }

        Node* nodePtr;  ///< Pointer to the current node.
    };

    /**
     * @brief A reverse iterator for the list.
     */
    using ReverseIterator = std::reverse_iterator<Iterator>;

    /**
     * @brief Returns an iterator to the beginning of the list.
     *
     * @return An iterator to the beginning of the list.
     */
    Iterator begin() noexcept { return Iterator(head_.get()); }

    /**
     * @brief Returns an iterator to the end of the list.
     *
     * @return An iterator to the end of the list.
     */
    Iterator end() noexcept { return Iterator(nullptr); }

    /**
     * @brief Returns a reverse iterator to the beginning of the reversed list.
     *
     * @return A reverse iterator to the beginning of the reversed list.
     */
    ReverseIterator rbegin() noexcept { return ReverseIterator(end()); }

    /**
     * @brief Returns a reverse iterator to the end of the reversed list.
     *
     * @return A reverse iterator to the end of the reversed list.
     */
    ReverseIterator rend() noexcept { return ReverseIterator(begin()); }

    /**
     * @brief Inserts an element at the specified position.
     *
     * @param pos The position to insert the element at.
     * @param value The value to insert.
     */
    void insert(Iterator pos, const T& value) {
        if (pos == begin()) {
            pushFront(value);
        } else if (pos == end()) {
            pushBack(value);
        } else {
            auto newNode = std::make_unique<Node>(value);
            Node* prevNode = pos.nodePtr->prev;
            newNode->prev = prevNode;
            newNode->next = std::move(prevNode->next);
            prevNode->next = std::move(newNode);
            pos.nodePtr->prev = prevNode->next.get();
            ++list_size_;
        }
    }

    /**
     * @brief Erases the element at the specified position.
     *
     * @param pos The position of the element to erase.
     * @return An iterator to the element following the erased element.
     */
    Iterator erase(Iterator pos) noexcept {
        if (pos == begin()) {
            popFront();
            return begin();
        }
        if (pos == --end()) {
            popBack();
            return end();
        }
        Node* prevNode = pos.nodePtr->prev;
        Node* nextNode = pos.nodePtr->next.get();
        prevNode->next = std::move(pos.nodePtr->next);
        if (nextNode)
            nextNode->prev = prevNode;
        --list_size_;
        return Iterator(nextNode);
    }

    /**
     * @brief Removes all elements with the specified value.
     *
     * @param value The value to remove.
     */
    void remove(const T& value) {
        for (auto it = begin(); it != end();) {
            if (*it == value) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief Removes consecutive duplicate elements from the list.
     */
    void unique() {
        for (auto it = begin(); it != end();) {
            auto nextIt = std::next(it);
            if (nextIt != end() && *it == *nextIt) {
                it = erase(nextIt);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief Sorts the elements in the list.
     */
    void sort() {
        if (size() <= 1) {
            return;
        }
        SmallList<T> temp;
        while (!empty()) {
            auto it = begin();
            T minVal = *it;
            for (auto curr = ++begin(); curr != end(); ++curr) {
                if (*curr < minVal) {
                    minVal = *curr;
                    it = curr;
                }
            }
            temp.pushBack(minVal);
            erase(it);
        }
        swap(temp);
    }

    /**
     * @brief Swaps the contents of this list with another list.
     *
     * @param other The list to swap contents with.
     */
    void swap(SmallList<T>& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(list_size_, other.list_size_);
    }

    /**
     * @brief Constructs an element in place at the end of the list.
     *
     * @tparam Args The types of the arguments to construct the element with.
     * @param args The arguments to construct the element with.
     */
    template <typename... Args>
    void emplaceBack(Args&&... args) {
        T value(std::forward<Args>(args)...);
        pushBack(value);
    }

    /**
     * @brief Constructs an element in place at the front of the list.
     *
     * @tparam Args The types of the arguments to construct the element with.
     * @param args The arguments to construct the element with.
     */
    template <typename... Args>
    void emplaceFront(Args&&... args) {
        T value(std::forward<Args>(args)...);
        pushFront(value);
    }

    /**
     * @brief Constructs an element in place at the specified position.
     *
     * @tparam Args The types of the arguments to construct the element with.
     * @param pos The position to construct the element at.
     * @param args The arguments to construct the element with.
     * @return An iterator to the constructed element.
     */
    template <typename... Args>
    Iterator emplace(Iterator pos, Args&&... args) {
        T value(std::forward<Args>(args)...);
        insert(pos, value);
        return Iterator(pos.nodePtr);
    }
};

}  // namespace atom::type

#endif  // ATOM_TYPE_SMALL_LIST_HPP
