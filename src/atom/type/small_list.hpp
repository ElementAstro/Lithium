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
#include "macro.hpp"

template <typename T>
class SmallList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        explicit Node(const T& value)
            : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head_;
    Node* tail_;
    size_t list_size_;

public:
    SmallList() : head_(nullptr), tail_(nullptr), list_size_(0) {}

    SmallList(std::initializer_list<T> init_list) : SmallList() {
        for (const auto& value : init_list) {
            pushBack(value);
        }
    }

    ~SmallList() { clear(); }

    void pushBack(const T& value) {
        Node* newNode = new Node(value);
        if (empty()) {
            head_ = tail_ = newNode;
        } else {
            tail_->next = newNode;
            newNode->prev = tail_;
            tail_ = newNode;
        }
        ++list_size_;
    }

    void pushFront(const T& value) {
        Node* newNode = new Node(value);
        if (empty()) {
            head_ = tail_ = newNode;
        } else {
            head_->prev = newNode;
            newNode->next = head_;
            head_ = newNode;
        }
        ++list_size_;
    }

    void popBack() {
        if (!empty()) {
            Node* temp = tail_;
            tail_ = tail_->prev;
            if (tail_) {
                tail_->next = nullptr;
            } else {
                head_ = nullptr;
            }
            delete temp;
            --list_size_;
        }
    }

    void popFront() {
        if (!empty()) {
            Node* temp = head_;
            head_ = head_->next;
            if (head_) {
                head_->prev = nullptr;
            } else {
                tail_ = nullptr;
            }
            delete temp;
            --list_size_;
        }
    }

    auto front() -> T& { return head_->data; }

    auto front() const -> const T& { return head_->data; }

    auto back() -> T& { return tail_->data; }

    auto back() const -> const T& { return tail_->data; }

    ATOM_NODISCARD auto empty() const -> bool { return list_size_ == 0; }

    ATOM_NODISCARD auto size() const -> size_t { return list_size_; }

    void clear() {
        while (!empty()) {
            popFront();
        }
    }

    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit Iterator(Node* ptr = nullptr) : nodePtr(ptr) {}

        auto operator++() -> Iterator& {
            nodePtr = nodePtr->next;
            return *this;
        }

        auto operator++(int) -> Iterator {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        auto operator--() -> Iterator& {
            nodePtr = nodePtr->prev;
            return *this;
        }

        auto operator--(int) -> Iterator {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        auto operator==(const Iterator& other) const -> bool {
            return nodePtr == other.nodePtr;
        }

        auto operator!=(const Iterator& other) const -> bool {
            return !(*this == other);
        }

        auto operator*() const -> T& { return nodePtr->data; }

        auto operator->() const -> T* { return &(nodePtr->data); }

        Node* nodePtr;
    };

    auto begin() -> Iterator { return Iterator(head_); }

    auto end() -> Iterator { return Iterator(nullptr); }

    void insert(Iterator pos, const T& value) {
        if (pos == begin()) {
            pushFront(value);
        } else if (pos == end()) {
            pushBack(value);
        } else {
            Node* newNode = new Node(value);
            Node* prevNode = pos.nodePtr->prev;
            newNode->prev = prevNode;
            newNode->next = pos.nodePtr;
            prevNode->next = newNode;
            pos.nodePtr->prev = newNode;
            ++list_size_;
        }
    }

    auto erase(Iterator pos) -> Iterator {
        if (pos == begin()) {
            popFront();
            return begin();
        }
        if (pos == --end()) {
            popBack();
            return end();
        }
        Node* prevNode = pos.nodePtr->prev;
        Node* nextNode = pos.nodePtr->next;
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
        delete pos.nodePtr;
        --list_size_;
        return Iterator(nextNode);
    }

    void remove(const T& value) {
        for (auto it = begin(); it != end();) {
            if (*it == value) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

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

    void swap(SmallList<T>& other) {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(list_size_, other.list_size_);
    }
};

#endif  // ATOM_TYPE_SMALL_LIST_HPP
