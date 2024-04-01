/*
 * eventstack_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-26

Description: A thread-safe stack data structure for managing events.

**************************************************/

template <typename T>
class EventStack<T>::EventStackImpl {
public:
    std::vector<T> events; /**< Vector to store events. */
};

template <typename T>
EventStack<T>::EventStack() : impl(std::make_unique<EventStackImpl>()) {}

template <typename T>
EventStack<T>::~EventStack() {
}

template <typename T>
void EventStack<T>::pushEvent(const T& event) {
    std::lock_guard<std::mutex> lock(mtx);
    impl->events.push_back(event);
}

template <typename T>
T EventStack<T>::popEvent() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!impl->events.empty()) {
        T event = impl->events.back();
        impl->events.pop_back();
        return event;
    } else {
        throw std::out_of_range("Event stack is empty.");
    }
}

template <typename T>
void EventStack<T>::printEvents() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Events in stack:" << std::endl;
    for (const T& event : impl->events) {
        std::cout << event << std::endl;
    }
}

template <typename T>
bool EventStack<T>::isEmpty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return impl->events.empty();
}

template <typename T>
size_t EventStack<T>::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return impl->events.size();
}

template <typename T>
void EventStack<T>::clearEvents() {
    std::lock_guard<std::mutex> lock(mtx);
    impl->events.clear();
}

template <typename T>
T EventStack<T>::peekTopEvent() const {
    std::lock_guard<std::mutex> lock(mtx);
    if (!impl->events.empty()) {
        return impl->events.back();
    } else {
        throw std::out_of_range("Event stack is empty.");
    }
}

template <typename T>
EventStack<T> EventStack<T>::copyStack() const {
    std::lock_guard<std::mutex> lock(mtx);
    EventStack<T> newStack;
    newStack.impl->events = impl->events;
    return newStack;
}

template <typename T>
void EventStack<T>::filterEvents(bool (*filterFunc)(const T&)) {
    std::lock_guard<std::mutex> lock(mtx);
    impl->events.erase(std::remove_if(impl->events.begin(), impl->events.end(), [&](const T& event) {
        return !filterFunc(event);
    }), impl->events.end());
}

template <typename T>
std::string EventStack<T>::serializeStack() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::string serializedStack;
    for (const T& event : impl->events) {
        serializedStack += event + ";";
    }
    return serializedStack;
}

template <typename T>
void EventStack<T>::deserializeStack(const std::string& serializedData) {
    std::lock_guard<std::mutex> lock(mtx);
    impl->events.clear();
    size_t pos = 0;
    size_t nextPos = 0;
    while ((nextPos = serializedData.find(";", pos)) != std::string::npos) {
        T event = serializedData.substr(pos, nextPos - pos);
        impl->events.push_back(event);
        pos = nextPos + 1;
    }
}
