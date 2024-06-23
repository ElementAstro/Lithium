#ifndef ATOM_ASNYC_SAFETYPE_TPP
#define ATOM_ASNYC_SAFETYPE_TPP

#include "safetype.hpp"

namespace atom::async {
template <typename T>
LockFreeStack<T>::Node::Node(T value_) : value(std::move(value_)) {}

// 构造函数
template <typename T>
LockFreeStack<T>::LockFreeStack() : head(nullptr) {}

// 析构函数
template <typename T>
LockFreeStack<T>::~LockFreeStack() {
    while (auto node = head.load(std::memory_order_relaxed)) {
        head.store(node->next.load(std::memory_order_relaxed),
                   std::memory_order_relaxed);
        delete node;
    }
}

// push 常量左值引用
template <typename T>
void LockFreeStack<T>::push(const T& value) {
    auto new_node = new Node(value);
    new_node->next = head.load(std::memory_order_relaxed);
    Node* expected = new_node->next.load(std::memory_order_relaxed);
    while (!head.compare_exchange_weak(expected, new_node,
                                       std::memory_order_release,
                                       std::memory_order_relaxed)) {
        new_node->next = expected;
    }
    approximateSize.fetch_add(1, std::memory_order_relaxed);
}

// push 右值引用
template <typename T>
void LockFreeStack<T>::push(T&& value) {
    auto new_node = new Node(std::move(value));
    new_node->next = head.load(std::memory_order_relaxed);
    Node* expected = new_node->next.load(std::memory_order_relaxed);
    while (!head.compare_exchange_weak(expected, new_node,
                                       std::memory_order_release,
                                       std::memory_order_relaxed)) {
        new_node->next = expected;
    }
    approximateSize.fetch_add(1, std::memory_order_relaxed);
}

// pop
template <typename T>
std::optional<T> LockFreeStack<T>::pop() {
    Node* old_head = head.load(std::memory_order_relaxed);
    while (old_head && !head.compare_exchange_weak(old_head, old_head->next,
                                                   std::memory_order_acquire,
                                                   std::memory_order_relaxed)) {
    }
    if (old_head) {
        T value = std::move(old_head->value);
        delete old_head;
        approximateSize.fetch_sub(1, std::memory_order_relaxed);
        return value;
    } else {
        return std::nullopt;
    }
}

// top
template <typename T>
std::optional<T> LockFreeStack<T>::top() const {
    Node* top_node = head.load(std::memory_order_relaxed);
    if (top.el) {
        return std::optional<T>(top_node->value);
    } else {
        return std::nullopt;
    }
}

// empty
template <typename T>
bool LockFreeStack<T>::empty() const {
    return head.load(std::memory_order_relaxed) == nullptr;
}

// size
template <typename T>
int LockFreeStack<T>::size() const {
    return approximateSize.load(std::memory_order_relaxed);
}
}  // namespace atom::async

#endif