#ifndef ATOM_META_VANY_HPP
#define ATOM_META_VANY_HPP

#include <array>
#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <typeinfo>

#include "atom/error/exception.hpp"
#include "atom/macro.hpp"

template <typename T, typename = void>
struct IsIterable : std::false_type {};

template <typename T>
struct IsIterable<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                 decltype(std::end(std::declval<T>()))>>
    : std::true_type {};

template <typename T>
inline constexpr bool K_IS_ITERABLE_V = IsIterable<T>::value;

namespace atom::meta {
class Any {
private:
    struct ATOM_ALIGNAS(64) VTable {
        void (*destroy)(void*);
        void (*copy)(const void*, void*);
        void (*move)(void*, void*);
        const std::type_info& (*type)();
        std::string (*toString)(const void*);
        size_t (*size)();
        void (*invoke)(const void*, const std::function<void(const void*)>&);
        void (*foreach)(const void*, const std::function<void(const Any&)>&);
    };

    template <typename T>
    static auto defaultToString(const void* ptr) -> std::string {
        if constexpr (std::is_same_v<T, std::string>) {
            return *static_cast<const std::string*>(ptr);
        } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(*static_cast<const T*>(ptr));
        } else {
            std::ostringstream oss;
            oss << "Object of type " << typeid(T).name();
            return oss.str();
        }
    }

    template <typename T>
    static void defaultInvoke(const void* ptr,
                              const std::function<void(const void*)>& func) {
        func(ptr);
    }

    template <typename T>
    static void defaultForeach(const void* ptr,
                               const std::function<void(const Any&)>& func) {
        if constexpr (K_IS_ITERABLE_V<T>) {
#pragma unroll
            for (const auto& item : *static_cast<const T*>(ptr)) {
                func(Any(item));
            }
        } else {
            THROW_INVALID_ARGUMENT("Type is not iterable");
        }
    }

    template <typename T>
    static constexpr VTable K_V_TABLE = {
        [](void* ptr) { static_cast<T*>(ptr)->~T(); },
        [](const void* src, void* dst) {
            new (dst) T(*static_cast<const T*>(src));
        },
        [](void* src, void* dst) {
            new (dst) T(std::move(*static_cast<T*>(src)));
        },
        []() -> const std::type_info& { return typeid(T); },
        &defaultToString<T>,
        []() -> size_t { return sizeof(T); },
        &defaultInvoke<T>,
        &defaultForeach<T>};

    static constexpr size_t kSmallObjectSize = 3 * sizeof(void*);
    union {
        alignas(std::max_align_t) std::array<char, kSmallObjectSize> storage;
        void* ptr;
    };
    const VTable* vptr_ = nullptr;
    bool is_small_ = true;

    template <typename T>
    static constexpr bool kIsSmallObject =
        sizeof(T) <= kSmallObjectSize &&
        std::is_nothrow_move_constructible_v<T>;

    auto getPtr() -> void* {
        return is_small_ ? static_cast<void*>(storage.data()) : ptr;
    }

    [[nodiscard]] auto getPtr() const -> const void* {
        return is_small_ ? static_cast<const void*>(storage.data()) : ptr;
    }

    template <typename T>
    auto as() -> T* {
        return static_cast<T*>(getPtr());
    }

    template <typename T>
    [[nodiscard]] auto as() const -> const T* {
        return static_cast<const T*>(getPtr());
    }

public:
    Any() noexcept : storage{} {}

    Any(const Any& other)
        : vptr_(other.vptr_), is_small_(other.is_small_), ptr(nullptr) {
        if (vptr_ != nullptr) {
            try {
                if (is_small_) {
                    // 小对象直接拷贝到storage
                    std::memcpy(storage.data(), other.getPtr(), vptr_->size());
                } else {
                    // 大对象需要动态分配内存
                    ptr = std::malloc(vptr_->size());
                    if (ptr == nullptr) {
                        throw std::bad_alloc();
                    }
                    // 拷贝数据
                    vptr_->copy(other.getPtr(), ptr);
                }
            } catch (...) {
                // 清理已分配的资源
                if (!is_small_ && ptr != nullptr) {
                    std::free(ptr);
                    ptr = nullptr;
                }
                vptr_ = nullptr;
                is_small_ = true;
                throw;  // 重新抛出异常
            }
        }
    }

    Any(Any&& other) noexcept : vptr_(other.vptr_), is_small_(other.is_small_) {
        if (vptr_ != nullptr) {
            if (is_small_) {
                vptr_->move(other.storage.data(), storage.data());
            } else {
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            other.vptr_ = nullptr;
        }
    }

    template <typename T>
    auto allocateAligned(size_t size, size_t alignment) -> T* {
        // 确保对齐值是2的幂
        if (alignment & (alignment - 1)) {
            alignment = std::bit_ceil(alignment);
        }

        // 确保对齐值至少等于sizeof(void*)
        alignment = std::max(alignment, sizeof(void*));

        void* ptr = std::aligned_alloc(alignment, size);
        if (!ptr) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }

    template <typename T>
    explicit Any(T&& value) {
        using ValueType = std::remove_cvref_t<T>;
        try {
            if constexpr (kIsSmallObject<ValueType>) {
                // 确保内存对齐
                alignas(std::max_align_t) char* addr = storage.data();
                new (addr) ValueType(std::forward<T>(value));
                is_small_ = true;
            } else {
                // 使用智能指针临时管理内存
                auto temp = allocateAligned<ValueType>(
                    sizeof(ValueType),
                    std::max(alignof(ValueType), alignof(std::max_align_t)));
                if (!temp) {
                    throw std::bad_alloc();
                }

                try {
                    new (temp) ValueType(std::forward<T>(value));
                    ptr = temp;
                    is_small_ = false;
                } catch (...) {
                    std::free(temp);
                    throw;
                }
            }

            // 确保vtable初始化在对象构造完成后
            if constexpr (std::is_trivially_destructible_v<ValueType>) {
                vptr_ = &K_V_TABLE<ValueType>;
            } else {
                static const VTable vtable = K_V_TABLE<ValueType>;
                vptr_ = &vtable;
            }
        } catch (...) {
            reset();  // 清理已分配的资源
            throw;
        }
    }

    ~Any() { reset(); }

    auto operator=(const Any& other) -> Any& {
        if (this != &other) {
            Any temp(other);
            swap(temp);
        }
        return *this;
    }

    auto operator=(Any&& other) noexcept -> Any& {
        if (this != &other) {
            reset();
            vptr_ = other.vptr_;
            is_small_ = other.is_small_;
            if (!is_small_) {
                ptr = other.ptr;
                other.ptr = nullptr;
            } else {
                std::memcpy(storage.data(), other.storage.data(),
                            kSmallObjectSize);
            }
            other.vptr_ = nullptr;
        }
        return *this;
    }

    void swap(Any& other) noexcept {
        std::swap(vptr_, other.vptr_);
        std::swap(is_small_, other.is_small_);
        std::swap(storage, other.storage);
        std::swap(ptr, other.ptr);
    }

    template <typename T>
    auto operator=(T&& value) -> Any& {
        *this = Any(std::forward<T>(value));
        return *this;
    }

    void reset() {
        if (vptr_ != nullptr) {
            vptr_->destroy(getPtr());
            if (!is_small_) {
                std::free(ptr);
            }
            vptr_ = nullptr;
            is_small_ = true;
        }
    }

    [[nodiscard]] auto hasValue() const noexcept -> bool {
        return vptr_ != nullptr;
    }

    [[nodiscard]] auto type() const -> const std::type_info& {
        if (!hasValue()) {
            throw std::bad_typeid();
        }
        return vptr_->type();
    }

    template <typename T>
    [[nodiscard]] auto is() const -> bool {
        return hasValue() && vptr_->type() == typeid(T);
    }

    template <typename T>
    auto cast() -> T& {
        if (!is<T>()) {
            throw std::bad_cast();
        }
        return *as<T>();
    }

    template <typename T>
    [[nodiscard]] auto cast() const -> const T& {
        if (!is<T>()) {
            throw std::bad_cast();
        }
        return *as<T>();
    }

    [[nodiscard]] auto toString() const -> std::string {
        if (!hasValue()) {
            return "Empty Any";
        }
        return vptr_->toString(getPtr());
    }

    void invoke(const std::function<void(const void*)>& func) const {
        if (!hasValue()) {
            THROW_INVALID_ARGUMENT("Cannot invoke on empty Any");
        }
        vptr_->invoke(getPtr(), func);
    }

    void foreach (const std::function<void(const Any&)>& func) const {
        if (!hasValue()) {
            THROW_INVALID_ARGUMENT("Cannot iterate over empty Any");
        }
        vptr_->foreach (getPtr(), func);
    }
};
}  // namespace atom::meta

#endif
