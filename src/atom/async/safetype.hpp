#ifndef ATOM_ASYNC_SAFETYPE_HPP
#define ATOM_ASYNC_SAFETYPE_HPP

#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
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

    std::atomic<Node*> head;  ///< Atomic pointer to the top of the stack.
    std::atomic<int> approximateSize =
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
    std::optional<T> pop();

    /**
     * @brief Get the top value of the stack without removing it. Thread-safe.
     *
     * @return std::optional<T> The top value if stack is not empty, otherwise
     * nullopt.
     */
    std::optional<T> top() const;

    /**
     * @brief Check if the stack is empty. Thread-safe.
     *
     * @return true If the stack is empty.
     * @return false If the stack has one or more elements.
     */
    bool empty() const;

    /**
     * @brief Get the approximate size of the stack. Thread-safe.
     *
     * @return int The approximate number of elements in the stack.
     */
    int size() const;
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

        std::optional<Value> find(const Key& key) const {
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
            Node* new_node = new Node(key, value);
            new_node->next = head.load();
            Node* expected = new_node->next.load();
            while (!head.compare_exchange_weak(expected, new_node)) {
                new_node->next = expected;
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

    std::vector<std::unique_ptr<Bucket>> buckets;
    std::hash<Key> hasher;

    Bucket& get_bucket(const Key& key) const {
        auto bucket_index = hasher(key) % buckets.size();
        return *buckets[bucket_index];
    }

public:
    LockFreeHashTable(size_t num_buckets = 16) : buckets(num_buckets) {
        for (size_t i = 0; i < num_buckets; ++i) {
            buckets[i] = std::make_unique<Bucket>();
        }
    }

    std::optional<Value> find(const Key& key) const {
        return get_bucket(key).find(key);
    }

    void insert(const Key& key, const Value& value) {
        get_bucket(key).insert(key, value);
    }

    void erase(const Key& key) { get_bucket(key).erase(key); }

    bool empty() const {
        for (const auto& bucket : buckets) {
            if (bucket->head.load() != nullptr) {
                return false;
            }
        }
        return true;
    }

    size_t size() const {
        size_t total_size = 0;
        for (const auto& bucket : buckets) {
            Node* node = bucket->head.load();
            while (node) {
                ++total_size;
                node = node->next.load();
            }
        }
        return total_size;
    }

    void clear() {
        for (const auto& bucket : buckets) {
            Node* node = bucket->head.load();
            while (node) {
                Node* next = node->next.load();
                delete node;
                node = next;
            }
            bucket->head.store(nullptr);
        }
    }

    // 迭代器类
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Key, Value>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(
            typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_iter,
            typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_end,
            Node* node)
            : bucket_iter(bucket_iter), bucket_end(bucket_end), node(node) {
            advance_past_empty_buckets();
        }

        iterator& operator++() {
            if (node) {
                node = node->next.load();
                if (!node) {
                    ++bucket_iter;
                    advance_past_empty_buckets();
                }
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return bucket_iter == other.bucket_iter && node == other.node;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        reference operator*() const {
            return *reinterpret_cast<value_type*>(node);
        }

        pointer operator->() const { return reinterpret_cast<pointer>(node); }

    private:
        void advance_past_empty_buckets() {
            while (bucket_iter != bucket_end && !node) {
                node = (*bucket_iter)->head.load();
                if (!node) {
                    ++bucket_iter;
                }
            }
        }

        typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_iter;
        typename std::vector<std::unique_ptr<Bucket>>::iterator bucket_end;
        Node* node;
    };

    iterator begin() {
        auto bucket_iter = buckets.begin();
        auto bucket_end = buckets.end();
        Node* node =
            bucket_iter != bucket_end ? (*bucket_iter)->head.load() : nullptr;
        return iterator(bucket_iter, bucket_end, node);
    }

    iterator end() { return iterator(buckets.end(), buckets.end(), nullptr); }
};

template <typename T>
class ThreadSafeVector {
private:
    std::atomic<T*> data;
    std::atomic<size_t> capacity;
    std::atomic<size_t> size;
    mutable std::shared_mutex resize_mutex;

    void resize() {
        std::unique_lock lock(resize_mutex);

        size_t old_capacity = capacity.load(std::memory_order_relaxed);
        size_t new_capacity = old_capacity * 2;
        T* new_data = new T[new_capacity];

        for (size_t i = 0; i < size.load(std::memory_order_relaxed); ++i) {
            new_data[i] = std::move(data.load(std::memory_order_relaxed)[i]);
        }

        T* old_data = data.exchange(new_data, std::memory_order_acq_rel);
        capacity.store(new_capacity, std::memory_order_release);

        delete[] old_data;
    }

public:
    explicit ThreadSafeVector(size_t initial_capacity = 16)
        : data(new T[initial_capacity]), capacity(initial_capacity), size(0) {}

    ~ThreadSafeVector() { delete[] data.load(std::memory_order_relaxed); }

    void push_back(const T& value) {
        size_t current_size = size.load(std::memory_order_relaxed);
        while (true) {
            if (current_size < capacity.load(std::memory_order_relaxed)) {
                if (size.compare_exchange_weak(current_size, current_size + 1,
                                               std::memory_order_acq_rel)) {
                    data.load(std::memory_order_relaxed)[current_size] = value;
                    return;
                }
            } else {
                resize();
            }
            current_size = size.load(std::memory_order_relaxed);
        }
    }

    void push_back(T&& value) {
        size_t current_size = size.load(std::memory_order_relaxed);
        while (true) {
            if (current_size < capacity.load(std::memory_order_relaxed)) {
                if (size.compare_exchange_weak(current_size, current_size + 1,
                                               std::memory_order_acq_rel)) {
                    data.load(std::memory_order_relaxed)[current_size] =
                        std::move(value);
                    return;
                }
            } else {
                resize();
            }
            current_size = size.load(std::memory_order_relaxed);
        }
    }

    std::optional<T> pop_back() {
        size_t current_size = size.load(std::memory_order_relaxed);
        while (current_size > 0) {
            if (size.compare_exchange_weak(current_size, current_size - 1,
                                           std::memory_order_acq_rel)) {
                return data.load(std::memory_order_relaxed)[current_size - 1];
            }
            current_size = size.load(std::memory_order_relaxed);
        }
        return std::nullopt;
    }

    std::optional<T> at(size_t index) const {
        if (index >= size.load(std::memory_order_relaxed)) {
            return std::nullopt;
        }
        return data.load(std::memory_order_relaxed)[index];
    }

    bool empty() const { return size.load(std::memory_order_relaxed) == 0; }

    size_t getSize() const { return size.load(std::memory_order_relaxed); }

    size_t getCapacity() const {
        return capacity.load(std::memory_order_relaxed);
    }

    void clear() { size.store(0, std::memory_order_relaxed); }

    void shrink_to_fit() {
        std::unique_lock lock(resize_mutex);

        size_t current_size = size.load(std::memory_order_relaxed);
        T* new_data = new T[current_size];

        for (size_t i = 0; i < current_size; ++i) {
            new_data[i] = std::move(data.load(std::memory_order_relaxed)[i]);
        }

        T* old_data = data.exchange(new_data, std::memory_order_acq_rel);
        capacity.store(current_size, std::memory_order_release);

        delete[] old_data;
    }

    T front() const {
        if (empty())
            THROW_OUT_OF_RANGE("Vector is empty");
        return data.load(std::memory_order_relaxed)[0];
    }

    T back() const {
        if (empty())
            THROW_OUT_OF_RANGE("Vector is empty");
        return data.load(
            std::memory_order_relaxed)[size.load(std::memory_order_relaxed) -
                                       1];
    }

    T operator[](size_t index) const {
        if (index >= size.load(std::memory_order_relaxed))
            THROW_OUT_OF_RANGE("Index out of range");
        return data.load(std::memory_order_relaxed)[index];
    }
};

template <typename T>
class LockFreeList {
private:
    struct Node {
        std::shared_ptr<T> value;
        std::atomic<Node*> next;
        Node(T val) : value(std::make_shared<T>(val)), next(nullptr) {}
    };

    std::atomic<Node*> head;

    // Hazard pointers structure
    struct HazardPointer {
        std::atomic<std::thread::id> id;
        std::atomic<void*> pointer;
    };

    static const int MAX_HAZARD_POINTERS = 100;
    HazardPointer hazard_pointers[MAX_HAZARD_POINTERS];

    // Get hazard pointer for current thread
    std::atomic<void*>& get_hazard_pointer_for_current_thread() {
        std::thread::id this_id = std::this_thread::get_id();
        for (int i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            std::thread::id old_id;
            if (hazard_pointers[i].id.compare_exchange_strong(old_id, this_id)) {
                return hazard_pointers[i].pointer;
            } else if (hazard_pointers[i].id == this_id) {
                return hazard_pointers[i].pointer;
            }
        }
        THROW_RUNTIME_ERROR("No hazard pointers available");
    }

    // Reclaim list
    void reclaim_later(Node* node) {
        retired_nodes.push_back(node);
        if (retired_nodes.size() >= MAX_HAZARD_POINTERS) {
            do_reclamation();
        }
    }

    // Reclaim retired nodes
    void do_reclamation() {
        std::vector<Node*> to_reclaim;
        for (Node* node : retired_nodes) {
            if (!is_hazard(node)) {
                to_reclaim.push_back(node);
            }
        }
        retired_nodes.clear();
        for (Node* node : to_reclaim) {
            delete node;
        }
    }

    // Check if node is a hazard
    bool is_hazard(Node* node) {
        for (int i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            if (hazard_pointers[i].pointer.load() == node) {
                return true;
            }
        }
        return false;
    }

    std::vector<Node*> retired_nodes;

public:
    LockFreeList() : head(nullptr) {}

    ~LockFreeList() {
        while (head.load()) {
            Node* old_head = head.load();
            head.store(old_head->next);
            delete old_head;
        }
    }

    void push_front(T value) {
        Node* new_node = new Node(value);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node)) {}
    }

    std::optional<T> pop_front() {
        std::atomic<void*>& hazard_pointer = get_hazard_pointer_for_current_thread();
        Node* old_head = head.load();
        do {
            Node* temp;
            do {
                temp = old_head;
                hazard_pointer.store(old_head);
                old_head = head.load();
            } while (old_head != temp);
            if (!old_head) {
                hazard_pointer.store(nullptr);
                return std::nullopt;
            }
        } while (!head.compare_exchange_strong(old_head, old_head->next));
        hazard_pointer.store(nullptr);
        std::shared_ptr<T> res = old_head->value;
        if (res.use_count() == 1) {
            reclaim_later(old_head);
        }
        return *res;
    }

    bool empty() const {
        return head.load() == nullptr;
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(Node* node, LockFreeList* list) : node(node), list(list) {}

        iterator& operator++() {
            if (node) {
                node = node->next.load();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return node == other.node;
        }

        bool operator!=(const iterator& other) const {
            return node != other.node;
        }

        reference operator*() const {
            return *(node->value);
        }

        pointer operator->() const {
            return node->value.get();
        }

    private:
        Node* node;
        LockFreeList* list;
    };

    iterator begin() {
        return iterator(head.load(), this);
    }

    iterator end() {
        return iterator(nullptr, this);
    }
};
}  // namespace atom::async

#include "safetype.tpp"  // Include the implementation

#endif  // ATOM_ASYNC_SAFETYPE_HPP