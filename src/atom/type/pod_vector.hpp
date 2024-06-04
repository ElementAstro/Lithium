#ifndef ATOM_TYPE_POD_VECTOR_HPP
#define ATOM_TYPE_POD_VECTOR_HPP

#include <algorithm>
#include <concepts>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace atom::type {
template <typename T>
concept PodType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

void* pool64_alloc(std::size_t size) { return std::malloc(size); }

void pool64_dealloc(void* ptr) { std::free(ptr); }

template <PodType T, int Growth = 2>
struct pod_vector {
    static constexpr int SizeT = sizeof(T);
    static constexpr int N = 64 / SizeT;

    static_assert(N >= 4, "Element size too large");

    int _size;
    int _capacity;
    T* _data;

    using size_type = int;

    pod_vector() : _size(0), _capacity(N) {
        _data = static_cast<T*>(pool64_alloc(_capacity * SizeT));
    }

    pod_vector(std::initializer_list<T> il)
        : _size(il.size()), _capacity(std::max(N, _size)) {
        _data = static_cast<T*>(pool64_alloc(_capacity * SizeT));
        std::copy(il.begin(), il.end(), _data);
    }

    explicit pod_vector(int size) : _size(size), _capacity(std::max(N, size)) {
        _data = static_cast<T*>(pool64_alloc(_capacity * SizeT));
    }

    pod_vector(const pod_vector& other)
        : _size(other._size), _capacity(other._capacity) {
        _data = static_cast<T*>(pool64_alloc(_capacity * SizeT));
        std::memcpy(_data, other._data, SizeT * _size);
    }

    pod_vector(pod_vector&& other) noexcept
        : _size(other._size), _capacity(other._capacity), _data(other._data) {
        other._data = nullptr;
    }

    pod_vector& operator=(pod_vector&& other) noexcept {
        if (this != &other) {
            if (_data != nullptr)
                pool64_dealloc(_data);
            _size = other._size;
            _capacity = other._capacity;
            _data = other._data;
            other._data = nullptr;
        }
        return *this;
    }

    pod_vector& operator=(const pod_vector& other) = delete;

    template <typename __ValueT>
    void push_back(__ValueT&& t) {
        if (_size == _capacity)
            reserve(_capacity * Growth);
        _data[_size++] = std::forward<__ValueT>(t);
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (_size == _capacity)
            reserve(_capacity * Growth);
        new (&_data[_size++]) T(std::forward<Args>(args)...);
    }

    void reserve(int cap) {
        if (cap <= _capacity)
            return;
        _capacity = cap;
        T* old_data = _data;
        _data = static_cast<T*>(pool64_alloc(_capacity * SizeT));
        if (old_data != nullptr) {
            std::memcpy(_data, old_data, SizeT * _size);
            pool64_dealloc(old_data);
        }
    }

    void pop_back() { _size--; }
    T popx_back() {
        T t = std::move(_data[_size - 1]);
        _size--;
        return t;
    }

    void extend(const pod_vector& other) {
        for (const auto& elem : other)
            push_back(elem);
    }

    void extend(const T* begin, const T* end) {
        for (auto it = begin; it != end; ++it)
            push_back(*it);
    }

    T& operator[](int index) { return _data[index]; }
    const T& operator[](int index) const { return _data[index]; }

    T* begin() { return _data; }
    T* end() { return _data + _size; }
    const T* begin() const { return _data; }
    const T* end() const { return _data + _size; }
    T& back() { return _data[_size - 1]; }
    const T& back() const { return _data[_size - 1]; }

    bool empty() const { return _size == 0; }
    int size() const { return _size; }
    T* data() { return _data; }
    const T* data() const { return _data; }
    void clear() { _size = 0; }

    template <typename __ValueT>
    void insert(int i, __ValueT&& val) {
        if (_size == _capacity)
            reserve(_capacity * Growth);
        for (int j = _size; j > i; j--)
            _data[j] = _data[j - 1];
        _data[i] = std::forward<__ValueT>(val);
        _size++;
    }

    void erase(int i) {
        for (int j = i; j < _size - 1; j++)
            _data[j] = _data[j + 1];
        _size--;
    }

    void reverse() { std::reverse(_data, _data + _size); }

    void resize(int size) {
        if (size > _capacity)
            reserve(size);
        _size = size;
    }

    std::pair<T*, int> detach() noexcept {
        T* p = _data;
        int size = _size;
        _data = nullptr;
        _size = 0;
        return {p, size};
    }

    ~pod_vector() {
        if (_data != nullptr)
            pool64_dealloc(_data);
    }
};

template <typename T, typename Container = std::vector<T>>
class stack {
    Container vec;

public:
    void push(const T& t) { vec.push_back(t); }
    void push(T&& t) { vec.push_back(std::move(t)); }
    template <typename... Args>
    void emplace(Args&&... args) {
        vec.emplace_back(std::forward<Args>(args)...);
    }
    void pop() { vec.pop_back(); }
    void clear() { vec.clear(); }
    bool empty() const { return vec.empty(); }
    typename Container::size_type size() const { return vec.size(); }
    T& top() { return vec.back(); }
    const T& top() const { return vec.back(); }
    T popx() {
        T t = std::move(vec.back());
        vec.pop_back();
        return t;
    }
    void reserve(int n) { vec.reserve(n); }
    Container& container() { return vec; }
    const Container& container() const { return vec; }
};

}  // namespace atom::type

#endif
