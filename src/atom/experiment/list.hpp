/*
 * list.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: An implementation of deque. Using C++11 and C++17.

**************************************************/

#ifndef ATOM_EXPERIMENTAL_LIST_HPP
#define ATOM_EXPERIMENTAL_LIST_HPP

#include <iostream>
#include <optional>

template <typename T>
struct Node {
    T data;
    Node *prev;
    Node *next;

    Node(const T &value) : data(value), prev(nullptr), next(nullptr) {}
};

template <typename T>
class DequeIterator {
private:
    Node<T> *current;

public:
    DequeIterator(Node<T> *node) : current(node) {}

    bool operator==(const DequeIterator &other) const {
        return current == other.current;
    }

    bool operator!=(const DequeIterator &other) const {
        return !(*this == other);
    }

    T &operator*() const { return current->data; }

    DequeIterator &operator++() {
        if (current != nullptr) {
            current = current->next;
        }
        return *this;
    }

    DequeIterator operator++(int) {
        DequeIterator temp = *this;
        ++(*this);
        return temp;
    }

    DequeIterator &operator--() {
        if (current != nullptr) {
            current = current->prev;
        }
        return *this;
    }

    DequeIterator operator--(int) {
        DequeIterator temp = *this;
        --(*this);
        return temp;
    }
};

template <typename T>
class Deque {
private:
    Node<T> *head;
    Node<T> *tail;
    size_t size;

public:
    Deque() : head(nullptr), tail(nullptr), size(0) {}

    void push_front(const T &value) {
        Node<T> *newNode = new Node<T>(value);
        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    void push_back(const T &value) {
        Node<T> *newNode = new Node<T>(value);
        if (tail == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }

    std::optional<T> pop_front() {
        if (head == nullptr) {
            return {};
        }
        T value = head->data;
        Node<T> *temp = head;
        head = head->next;
        if (head != nullptr) {
            head->prev = nullptr;
        } else {
            tail = nullptr;
        }
        delete temp;
        size--;
        return value;
    }

    std::optional<T> pop_back() {
        if (tail == nullptr) {
            return {};
        }
        T value = tail->data;
        Node<T> *temp = tail;
        tail = tail->prev;
        if (tail != nullptr) {
            tail->next = nullptr;
        } else {
            head = nullptr;
        }
        delete temp;
        size--;
        return value;
    }

    std::optional<T> peek_front() const {
        if (head == nullptr) {
            return {};
        }
        return head->data;
    }

    std::optional<T> peek_back() const {
        if (tail == nullptr) {
            return {};
        }
        return tail->data;
    }

    size_t get_size() const { return size; }

    void clear() {
        while (head != nullptr) {
            Node<T> *temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        size = 0;
    }

    bool empty() const { return head == nullptr; }

    // 查找元素，返回第一次出现的位置（从头向尾）
    std::optional<size_t> find(const T &value) {
        Node<T> *current = head;
        size_t index = 0;
        while (current != nullptr) {
            if (current->data == value) {
                return index;
            }
            current = current->next;
            index++;
        }
        return {};
    }

    // 在指定位置插入元素
    void insert(size_t index, const T &value) {
        if (index > size) {
            return;  // 超出范围
        }
        if (index == 0) {
            push_front(value);
        } else if (index == size) {
            push_back(value);
        } else {
            Node<T> *current = head;
            for (size_t i = 0; i < index; i++) {
                current = current->next;
            }
            Node<T> *newNode = new Node<T>(value);
            newNode->prev = current->prev;
            newNode->next = current;
            current->prev->next = newNode;
            current->prev = newNode;
            size++;
        }
    }

    // 移除指定位置的元素
    void remove_at(size_t index) {
        if (index >= size) {
            return;  // 超出范围
        }
        Node<T> *current = head;
        for (size_t i = 0; i < index; i++) {
            current = current->next;
        }
        if (current == head) {
            pop_front();
        } else if (current == tail) {
            pop_back();
        } else {
            current->prev->next = current->next;
            current->next->prev = current->prev;
            delete current;
            size--;
        }
    }

    // 反向遍历链表
    void reverse_traversal() {
        Node<T> *current = tail;
        while (current != nullptr) {
            std::cout << current->data << " ";
            current = current->prev;
        }
        std::cout << std::endl;
    }

    // 将另一个链表连接到当前链表后面
    void concatenate(Deque &other) {
        if (other.empty()) {
            return;
        }
        if (empty()) {
            head = other.head;
            tail = other.tail;
        } else {
            tail->next = other.head;
            other.head->prev = tail;
            tail = other.tail;
        }
        size += other.get_size();
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    DequeIterator<T> begin() const { return DequeIterator<T>(head); }

    DequeIterator<T> end() const { return DequeIterator<T>(nullptr); }
};

#endif
