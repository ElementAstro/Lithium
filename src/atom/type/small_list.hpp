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

template <typename T>
class small_list {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;

        Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    size_t list_size;

public:
    small_list() : head(nullptr), tail(nullptr), list_size(0) {}

    small_list(std::initializer_list<T> init_list) : small_list() {
        for (const auto& value : init_list) {
            push_back(value);
        }
    }

    ~small_list() { clear(); }

    void push_back(const T& value) {
        Node* new_node = new Node(value);
        if (empty()) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            new_node->prev = tail;
            tail = new_node;
        }
        ++list_size;
    }

    void push_front(const T& value) {
        Node* new_node = new Node(value);
        if (empty()) {
            head = tail = new_node;
        } else {
            head->prev = new_node;
            new_node->next = head;
            head = new_node;
        }
        ++list_size;
    }

    void pop_back() {
        if (!empty()) {
            Node* temp = tail;
            tail = tail->prev;
            if (tail) {
                tail->next = nullptr;
            } else {
                head = nullptr;
            }
            delete temp;
            --list_size;
        }
    }

    void pop_front() {
        if (!empty()) {
            Node* temp = head;
            head = head->next;
            if (head) {
                head->prev = nullptr;
            } else {
                tail = nullptr;
            }
            delete temp;
            --list_size;
        }
    }

    T& front() { return head->data; }

    const T& front() const { return head->data; }

    T& back() { return tail->data; }

    const T& back() const { return tail->data; }

    bool empty() const { return list_size == 0; }

    size_t size() const { return list_size; }

    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(Node* ptr = nullptr) : node_ptr(ptr) {}

        iterator& operator++() {
            node_ptr = node_ptr->next;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        iterator& operator--() {
            node_ptr = node_ptr->prev;
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return node_ptr == other.node_ptr;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        T& operator*() const { return node_ptr->data; }

        T* operator->() const { return &(node_ptr->data); }

        Node* node_ptr;
    };

    iterator begin() { return iterator(head); }

    iterator end() { return iterator(nullptr); }

    void insert(iterator pos, const T& value) {
        if (pos == begin()) {
            push_front(value);
        } else if (pos == end()) {
            push_back(value);
        } else {
            Node* new_node = new Node(value);
            Node* prev_node = pos.node_ptr->prev;
            new_node->prev = prev_node;
            new_node->next = pos.node_ptr;
            prev_node->next = new_node;
            pos.node_ptr->prev = new_node;
            ++list_size;
        }
    }

    iterator erase(iterator pos) {
        if (pos == begin()) {
            pop_front();
            return begin();
        } else if (pos == --end()) {
            pop_back();
            return end();
        } else {
            Node* prev_node = pos.node_ptr->prev;
            Node* next_node = pos.node_ptr->next;
            prev_node->next = next_node;
            next_node->prev = prev_node;
            delete pos.node_ptr;
            --list_size;
            return iterator(next_node);
        }
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
            auto next_it = std::next(it);
            if (next_it != end() && *it == *next_it) {
                it = erase(next_it);
            } else {
                ++it;
            }
        }
    }

    void sort() {
        if (size() <= 1) {
            return;
        }
        small_list<T> temp;
        while (!empty()) {
            auto it = begin();
            T min_val = *it;
            for (auto curr = ++begin(); curr != end(); ++curr) {
                if (*curr < min_val) {
                    min_val = *curr;
                    it = curr;
                }
            }
            temp.push_back(min_val);
            erase(it);
        }
        swap(temp);
    }

    void swap(small_list<T>& other) {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(list_size, other.list_size);
    }
};

#endif  // ATOM_TYPE_SMALL_LIST_HPP
