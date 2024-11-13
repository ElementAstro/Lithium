#ifndef ATOM_ASYNC_SAFETYPE_HPP
#define ATOM_ASYNC_SAFETYPE_HPP

#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::async {
/**
 * @brief A lock-free stack implementation suitable for concurrent use.
 *
 * @tparam T Type of elements stored in the stack.
 */
template <typename T>
class LockFreeStack {
private:
    struct Node {
        T value;  ///< The stored value of type T.
        std::atomic<Node*> next =
            nullptr;  ///< Pointer to the next node in the stack.

        /**
         * @brief Construct a new Node object.
         *
         * @param value_ The value to store in the node.
         */
        explicit Node(T value_);
    };

    std::atomic<Node*> head_;  ///< Atomic pointer to the top of the stack.
    std::atomic<int> approximateSize_ =
        0;  ///< An approximate count of the stack's elements.

public:
    /**
     * @brief Construct a new Lock Free Stack object.
     */
    LockFreeStack();

    /**
     * @brief Destroy the Lock Free Stack object.
     */
    ~LockFreeStack();

    /**
     * @brief Pushes a value onto the stack. Thread-safe.
     *
     * @param value The value to push onto the stack.
     */
    void push(const T& value);

    /**
     * @brief Pushes a value onto the stack using move semantics. Thread-safe.
     *
     * @param value The value to move onto the stack.
     */
    void push(T&& value);

    /**
     * @brief Attempts to pop the top value off the stack. Thread-safe.
     *
     * @return std::optional<T> The popped value if stack is not empty,
     * otherwise nullopt.
     */
    auto pop() -> std::optional<T>;

    /**
     * @brief Get the top value of the stack without removing it. Thread-safe.
     *
     * @return std::optional<T> The top value if stack is not empty, otherwise
     * nullopt.
     */
    auto top() const -> std::optional<T>;

    /**
     * @brief Check if the stack is empty. Thread-safe.
     *
     * @return true If the stack is empty.
     * @return false If the stack has one or more elements.
     */
    [[nodiscard]] auto empty() const -> bool;

    /**
     * @brief Get the approximate size of the stack. Thread-safe.
     *
     * @return int The approximate number of elements in the stack.
     */
    [[nodiscard]] auto size() const -> int;
};

template <typename Key, typename Value>
class LockFreeHashTable {
private:
    struct Node {
        Key key;
        Value value;
        std::atomic<Node*> next;

        Node(Key k, Value v) : key(k), value(v), next(nullptr) {}
    };

    struct Bucket {
        std::atomic<Node*> head;

        Bucket() : head(nullptr) {}

        ~Bucket() {
            Node* node = head.load();
            while (node) {
                Node* next = node->next.load();
                delete node;
                node = next;
            }
        }

        auto find(const Key& key) const -> std::optional<Value> {
            Node* node = head.load();
            while (node) {
                if (node->key == key) {
                    return node->value;
                }
                node = node->next.load();
            }
            return std::nullopt;
        }

        void insert(const Key& key, const Value& value) {
            Node* newNode = new Node(key, value);
            newNode->next = head.load();
            Node* expected = newNode->next.load();
            while (!head.compare_exchange_weak(expected, newNode)) {
                newNode->next = expected;
            }
        }

        void erase(const Key& key) {
            Node* node = head.load();
            Node* prev = nullptr;
            while (node) {
                if (node->key == key) {
                    Node* next = node->next.load();
                    if (prev) {
                        prev->next.compare_exchange_strong(node, next);
                    } else {
                        head.compare_exchange_strong(node, next);
                    }
                    delete node;
                    return;
                }
                prev = node;
                node = node->next.load();
            }
        }
    };

    std::vector<std::unique_ptr<Bucket>> buckets_;
    std::hash<Key> hasher_;

    auto getBucket(const Key& key) const -> Bucket& {
        auto bucketIndex = hasher_(key) % buckets_.size();
        return *buckets_[bucketIndex];
    }

public:
    explicit LockFreeHashTable(size_t num_buckets = 16)
        : buckets_(num_buckets) {
        for (size_t i = 0; i < num_buckets; ++i) {
            buckets_[i] = std::make_unique<Bucket>();
        }
    }

    auto find(const Key& key) const -> std::optional<Value> {
        return getBucket(key).find(key);
    }

    void insert(const Key& key, const Value& value) {
        getBucket(key).insert(key, value);
    }

    void erase(const Key& key) { getBucket(key).erase(key); }

    [[nodiscard]] auto empty() const -> bool {
        for (const auto& bucket : buckets_) {
            if (bucket->head.load() != nullptr) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto size() const -> size_t {
        size_t totalSize = 0;
        for (const auto& bucket : buckets_) {
            Node* node = bucket->head.load();
            while (node) {
                ++totalSize;
                node = node->next.load();
            }
        }
        return totalSize;
    }

    void clear() {
        for (const auto& bucket : buckets_) {
            Node* node = bucket->head.load();
            while (node) {
                Node* next = node->next.load();
                delete node;
                node = next;
            }
            bucket->head.store(nullptr);
        }
    }

    auto operator[](const Key& key) -> Value& {
        auto& bucket = getBucket(key);
        auto value = bucket.find(key);
        if (value) {
            return *value;
        }
        insert(key, Value());
        return *find(key);
    }

    // 迭代器类
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Key, Value>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(
            typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_iter,
            typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_end,
            Node* node)
            : bucket_iter_(bucket_iter), bucket_end_(bucket_end), node_(node) {
            advancePastEmptyBuckets();
        }

        auto operator++() -> Iterator& {
            if (node_) {
                node_ = node_->next.load();
                if (!node_) {
                    ++bucket_iter_;
                    advancePastEmptyBuckets();
                }
            }
            return *this;
        }

        auto operator++(int) -> Iterator {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const Iterator& other) const -> bool {
            return bucket_iter_ == other.bucket_iter_ && node_ == other.node_;
        }

        auto operator!=(const Iterator& other) const -> bool {
            return !(*this == other);
        }

        auto operator*() const -> reference {
            return *reinterpret_cast<value_type*>(node_);
        }

        auto operator->() const -> pointer {
            return reinterpret_cast<pointer>(node_);
        }

    private:
        void advancePastEmptyBuckets() {
            while (bucket_iter_ != bucket_end_ && !node_) {
                node_ = (*bucket_iter_)->head.load();
                if (!node_) {
                    ++bucket_iter_;
                }
            }
        }

        typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_iter_;
        typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_end_;
        Node* node_;
    };

    auto begin() -> Iterator {
        auto bucketIter = buckets_.begin();
        auto bucketEnd = buckets_.end();
        Node* node =
            bucketIter != bucketEnd ? (*bucketIter)->head.load() : nullptr;
        return Iterator(bucketIter, bucketEnd, node);
    }

    auto end() -> Iterator {
        return Iterator(buckets_.end(), buckets_.end(), nullptr);
    }
};

template <typename T>
class ThreadSafeVector {
    std::atomic<T*> data_;
    std::atomic<size_t> capacity_;
    std::atomic<size_t> size_;
    mutable std::shared_mutex resize_mutex_;

    void resize() {
        std::unique_lock lock(resize_mutex_);

        size_t oldCapacity = capacity_.load(std::memory_order_relaxed);
        size_t newCapacity = oldCapacity * 2;
        T* newData = new T[newCapacity];

        for (size_t i = 0; i < size_.load(std::memory_order_relaxed); ++i) {
            newData[i] = std::move(data_.load(std::memory_order_relaxed)[i]);
        }

        T* oldData = data_.exchange(newData, std::memory_order_acq_rel);
        capacity_.store(newCapacity, std::memory_order_release);

        delete[] oldData;
    }

public:
    explicit ThreadSafeVector(size_t initial_capacity = 16)
        : data_(new T[initial_capacity]),
          capacity_(initial_capacity),
          size_(0) {}

    ~ThreadSafeVector() { delete[] data_.load(std::memory_order_relaxed); }

    void pushBack(const T& value) {
        size_t currentSize = size_.load(std::memory_order_relaxed);
        while (true) {
            if (currentSize < capacity_.load(std::memory_order_relaxed)) {
                if (size_.compare_exchange_weak(currentSize, currentSize + 1,
                                                std::memory_order_acq_rel)) {
                    data_.load(std::memory_order_relaxed)[currentSize] = value;
                    return;
                }
            } else {
                resize();
            }
            currentSize = size_.load(std::memory_order_relaxed);
        }
    }

    void pushBack(T&& value) {
        size_t currentSize = size_.load(std::memory_order_relaxed);
        while (true) {
            if (currentSize < capacity_.load(std::memory_order_relaxed)) {
                if (size_.compare_exchange_weak(currentSize, currentSize + 1,
                                                std::memory_order_acq_rel)) {
                    data_.load(std::memory_order_relaxed)[currentSize] =
                        std::move(value);
                    return;
                }
            } else {
                resize();
            }
            currentSize = size_.load(std::memory_order_relaxed);
        }
    }

    auto popBack() -> std::optional<T> {
        size_t currentSize = size_.load(std::memory_order_relaxed);
        while (currentSize > 0) {
            if (size_.compare_exchange_weak(currentSize, currentSize - 1,
                                            std::memory_order_acq_rel)) {
                return data_.load(std::memory_order_relaxed)[currentSize - 1];
            }
            currentSize = size_.load(std::memory_order_relaxed);
        }
        return std::nullopt;
    }

    auto at(size_t index) const -> std::optional<T> {
        if (index >= size_.load(std::memory_order_relaxed)) {
            return std::nullopt;
        }
        return data_.load(std::memory_order_relaxed)[index];
    }

    auto empty() const -> bool {
        return size_.load(std::memory_order_relaxed) == 0;
    }

    auto getSize() const -> size_t {
        return size_.load(std::memory_order_relaxed);
    }

    auto getCapacity() const -> size_t {
        return capacity_.load(std::memory_order_relaxed);
    }

    void clear() { size_.store(0, std::memory_order_relaxed); }

    void shrinkToFit() {
        std::unique_lock lock(resize_mutex_);

        size_t currentSize = size_.load(std::memory_order_relaxed);
        T* newData = new T[currentSize];

        for (size_t i = 0; i < currentSize; ++i) {
            newData[i] = std::move(data_.load(std::memory_order_relaxed)[i]);
        }

        T* oldData = data_.exchange(newData, std::memory_order_acq_rel);
        capacity_.store(currentSize, std::memory_order_release);

        delete[] oldData;
    }

    auto front() const -> T {
        if (empty()) {
            THROW_OUT_OF_RANGE("Vector is empty");
        }
        return data_.load(std::memory_order_relaxed)[0];
    }

    auto back() const -> T {
        if (empty()) {
            THROW_OUT_OF_RANGE("Vector is empty");
        }
        return data_.load(
            std::memory_order_relaxed)[size_.load(std::memory_order_relaxed) -
                                       1];
    }

    auto operator[](size_t index) const -> T {
        if (index >= size_.load(std::memory_order_relaxed)) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        return data_.load(std::memory_order_relaxed)[index];
    }
};

template <typename T>
class LockFreeList {
private:
    struct Node {
        std::shared_ptr<T> value;
        std::atomic<Node*> next;
        explicit Node(T val) : value(std::make_shared<T>(val)), next(nullptr) {}
    };

    std::atomic<Node*> head_;

    // Hazard pointers structure
    struct HazardPointer {
        std::atomic<std::thread::id> id;
        std::atomic<void*> pointer;
    };

    static const int MAX_HAZARD_POINTERS = 100;
    HazardPointer hazard_pointers_[MAX_HAZARD_POINTERS];

    // Get hazard pointer for current thread
    auto getHazardPointerForCurrentThread() -> std::atomic<void*>& {
        std::thread::id thisId = std::this_thread::get_id();
        for (auto& hazardPointer : hazard_pointers_) {
            std::thread::id oldId;
            if (hazardPointer.id.compare_exchange_strong(oldId, thisId)) {
                return hazardPointer.pointer;
            }
            if (hazardPointer.id == thisId) {
                return hazardPointer.pointer;
            }
        }
        THROW_RUNTIME_ERROR("No hazard pointers available");
    }

    // Reclaim list
    void reclaimLater(Node* node) {
        retired_nodes_.push_back(node);
        if (retired_nodes_.size() >= MAX_HAZARD_POINTERS) {
            doReclamation();
        }
    }

    // Reclaim retired nodes
    void doReclamation() {
        std::vector<Node*> toReclaim;
        for (Node* node : retired_nodes_) {
            if (!isHazard(node)) {
                toReclaim.push_back(node);
            }
        }
        retired_nodes_.clear();
        for (Node* node : toReclaim) {
            delete node;
        }
    }

    // Check if node is a hazard
    auto isHazard(Node* node) -> bool {
        for (auto& hazardPointer : hazard_pointers_) {
            if (hazardPointer.pointer.load() == node) {
                return true;
            }
        }
        return false;
    }

    std::vector<Node*> retired_nodes_;

public:
    LockFreeList() : head_(nullptr) {}

    ~LockFreeList() {
        while (head_.load()) {
            Node* oldHead = head_.load();
            head_.store(oldHead->next);
            delete oldHead;
        }
    }

    void pushFront(T value) {
        Node* newNode = new Node(value);
        newNode->next = head_.load();
        while (!head_.compare_exchange_weak(newNode->next, newNode)) {
        }
    }

    auto popFront() -> std::optional<T> {
        std::atomic<void*>& hazardPointer = getHazardPointerForCurrentThread();
        Node* oldHead = head_.load();
        do {
            Node* temp;
            do {
                temp = oldHead;
                hazardPointer.store(oldHead);
                oldHead = head_.load();
            } while (oldHead != temp);
            if (!oldHead) {
                hazardPointer.store(nullptr);
                return std::nullopt;
            }
        } while (!head_.compare_exchange_strong(oldHead, oldHead->next));
        hazardPointer.store(nullptr);
        std::shared_ptr<T> res = oldHead->value;
        if (res.use_count() == 1) {
            reclaimLater(oldHead);
        }
        return *res;
    }

    [[nodiscard]] auto empty() const -> bool { return head_.load() == nullptr; }

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(Node* node, LockFreeList* list) : node_(node), list_(list) {}

        auto operator++() -> Iterator& {
            if (node_) {
                node_ = node_->next.load();
            }
            return *this;
        }

        auto operator++(int) -> Iterator {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const Iterator& other) const -> bool {
            return node_ == other.node_;
        }

        auto operator!=(const Iterator& other) const -> bool {
            return node_ != other.node_;
        }

        auto operator*() const -> reference { return *(node_->value); }

        auto operator->() const -> pointer { return node_->value.get(); }

    private:
        Node* node_;
        LockFreeList* list_;
    };

    auto begin() -> Iterator { return Iterator(head_.load(), this); }

    auto end() -> Iterator { return Iterator(nullptr, this); }
};

template <typename T>
LockFreeStack<T>::Node::Node(T value_) : value(std::move(value_)) {}

// 构造函数
template <typename T>
LockFreeStack<T>::LockFreeStack() : head_(nullptr) {}

// 析构函数
template <typename T>
LockFreeStack<T>::~LockFreeStack() {
    while (auto node = head_.load(std::memory_order_relaxed)) {
        head_.store(node->next.load(std::memory_order_relaxed),
                    std::memory_order_relaxed);
        delete node;
    }
}

// push 常量左值引用
template <typename T>
void LockFreeStack<T>::push(const T& value) {
    auto newNode = new Node(value);
    newNode->next = head_.load(std::memory_order_relaxed);
    Node* expected = newNode->next.load(std::memory_order_relaxed);
    while (!head_.compare_exchange_weak(expected, newNode,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        newNode->next = expected;
    }
    approximateSize_.fetch_add(1, std::memory_order_relaxed);
}

// push 右值引用
template <typename T>
void LockFreeStack<T>::push(T&& value) {
    auto newNode = new Node(std::move(value));
    newNode->next = head_.load(std::memory_order_relaxed);
    Node* expected = newNode->next.load(std::memory_order_relaxed);
    while (!head_.compare_exchange_weak(expected, newNode,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        newNode->next = expected;
    }
    approximateSize_.fetch_add(1, std::memory_order_relaxed);
}

// pop
template <typename T>
auto LockFreeStack<T>::pop() -> std::optional<T> {
    Node* oldHead = head_.load(std::memory_order_relaxed);
    while (oldHead && !head_.compare_exchange_weak(oldHead, oldHead->next,
                                                   std::memory_order_acquire,
                                                   std::memory_order_relaxed)) {
    }
    if (oldHead) {
        T value = std::move(oldHead->value);
        delete oldHead;
        approximateSize_.fetch_sub(1, std::memory_order_relaxed);
        return value;
    }
    return std::nullopt;
}

// top
template <typename T>
auto LockFreeStack<T>::top() const -> std::optional<T> {
    Node* topNode = head_.load(std::memory_order_relaxed);
    if (head_.load(std::memory_order_relaxed)) {
        return std::optional<T>(topNode->value);
    }
    return std::nullopt;
}

// empty
template <typename T>
auto LockFreeStack<T>::empty() const -> bool {
    return head_.load(std::memory_order_relaxed) == nullptr;
}

// size
template <typename T>
auto LockFreeStack<T>::size() const -> int {
    return approximateSize_.load(std::memory_order_relaxed);
}
}  // namespace atom::async

#endif  // ATOM_ASYNC_SAFETYPE_HPP
