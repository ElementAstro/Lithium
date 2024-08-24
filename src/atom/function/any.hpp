/*!
 * \file any.hpp
 * \brief Enhanced BoxedValue using C++20 features
 * \author Max Qian <lightapt.com>
 * \date 2023-12-28
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ANY_HPP
#define ATOM_META_ANY_HPP

#include <any>
#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#include "macro.hpp"
#include "type_info.hpp"

namespace atom::meta {
class BoxedValue {
public:
    struct VoidType {};

private:
    struct Data {
        std::any mObj;
        TypeInfo mTypeInfo;
        std::shared_ptr<std::map<std::string, std::shared_ptr<Data>>> mAttrs;
        bool mIsRef = false;
        bool mReturnValue = false;
        bool mReadonly = false;
        const void* mConstDataPtr = nullptr;

        template <typename T>
            requires(!std::is_same_v<std::decay_t<T>, VoidType>)
        Data(T&& obj, bool is_ref, bool return_value, bool readonly)
            : mObj(std::forward<T>(obj)),
              mTypeInfo(userType<std::decay_t<T>>()),
              mAttrs(nullptr),
              mIsRef(is_ref),
              mReturnValue(return_value),
              mReadonly(readonly),
              mConstDataPtr(std::is_const_v<std::remove_reference_t<T>>
                                ? &obj
                                : nullptr) {}

        template <typename T>
            requires(std::is_same_v<std::decay_t<T>, VoidType>)
        Data(T&& obj, bool is_ref, bool return_value, bool readonly)
            : mTypeInfo(userType<std::decay_t<T>>()),
              mAttrs(nullptr),
              mIsRef(is_ref),
              mReturnValue(return_value),
              mReadonly(readonly),
              mConstDataPtr(std::is_const_v<std::remove_reference_t<T>>
                                ? &obj
                                : nullptr) {}
    } ATOM_ALIGNAS(128);

    std::shared_ptr<Data> m_data_;
    mutable std::shared_mutex m_mutex_;

public:
    // Constructor
    template <typename T>
        requires(!std::same_as<BoxedValue, std::decay_t<T>>)
    explicit BoxedValue(T&& t, bool t_return_value = false,
                        bool readonly = false)
        : m_data_(std::make_shared<Data>(
              std::forward<T>(t),
              std::is_reference_v<T> ||
                  std::is_same_v<
                      std::decay_t<T>,
                      std::reference_wrapper<std::remove_reference_t<T>>>,
              t_return_value, readonly)) {
        if constexpr (std::is_same_v<
                          std::decay_t<T>,
                          std::reference_wrapper<std::remove_reference_t<T>>>) {
            m_data_->mIsRef = true;
        }
    }

    // Default constructor for void type
    BoxedValue()
        : m_data_(std::make_shared<Data>(VoidType{}, false, false, false)) {}

    explicit BoxedValue(std::shared_ptr<Data> data)
        : m_data_(std::move(data)) {}

    // Copy constructor
    BoxedValue(const BoxedValue& other) {
        std::shared_lock lock(other.m_mutex_);
        if (other.m_data_) {
            m_data_ = std::make_shared<Data>(*other.m_data_);  // 深拷贝Data对象
        } else {
            m_data_ = nullptr;
        }
    }

    // Move constructor
    BoxedValue(BoxedValue&& other) noexcept {
        std::unique_lock lock(other.m_mutex_);
        m_data_ = std::move(other.m_data_);  // 安全移动
        other.m_data_ = nullptr;             // 清空原对象
    }

    // Copy assignment operator
    auto operator=(const BoxedValue& other) -> BoxedValue& {
        if (this != &other) {
            std::unique_lock lock(m_mutex_);
            std::shared_lock otherLock(other.m_mutex_);
            m_data_ = other.m_data_;
        }
        return *this;
    }

    // Move assignment operator
    auto operator=(BoxedValue&& other) noexcept -> BoxedValue& {
        if (this != &other) {
            std::unique_lock lock(m_mutex_);
            std::unique_lock otherLock(other.m_mutex_);
            m_data_ = std::move(other.m_data_);
        }
        return *this;
    }

    template <typename T>
        requires(!std::same_as<BoxedValue, std::decay_t<T>>)
    auto operator=(T&& t) -> BoxedValue& {
        std::unique_lock lock(m_mutex_);
        m_data_->mObj = std::forward<T>(t);
        m_data_->mTypeInfo = userType<T>();
        return *this;
    }

    template <typename T>
    BoxedValue(const T& t)
        : m_data_(std::make_shared<Data>(t, false, false, true)) {}

    // Swap function
    void swap(BoxedValue& rhs) noexcept {
        if (this != &rhs) {
            std::scoped_lock lock(m_mutex_, rhs.m_mutex_);
            std::swap(m_data_, rhs.m_data_);
        }
    }

    // Check if the value is undefined (VoidType)
    [[nodiscard]] bool isUndef() const noexcept {
        std::shared_lock lock(m_mutex_);
        if (m_data_ == nullptr) {
            return true;
        }
        return m_data_->mObj.type() == typeid(VoidType) ||
               !m_data_->mObj.has_value();
    }

    [[nodiscard]] bool isConst() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo.isConst();
    }

    [[nodiscard]] bool isType(const TypeInfo& ti) const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo == ti;
    }

    [[nodiscard]] bool isRef() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mIsRef;
    }

    [[nodiscard]] bool isReturnValue() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mReturnValue;
    }

    void resetReturnValue() noexcept {
        std::unique_lock lock(m_mutex_);
        m_data_->mReturnValue = false;
    }

    [[nodiscard]] bool isReadonly() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mReadonly;
    }

    [[nodiscard]] bool isConstDataPtr() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mConstDataPtr != nullptr;
    }

    [[nodiscard]] const std::any& get() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mObj;
    }

    [[nodiscard]] const TypeInfo& getTypeInfo() const noexcept {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo;
    }

    // Attribute handling
    BoxedValue& setAttr(const std::string& name, const BoxedValue& value) {
        std::unique_lock lock(m_mutex_);
        if (!m_data_->mAttrs) {
            m_data_->mAttrs = std::make_shared<
                std::map<std::string, std::shared_ptr<Data>>>();
        }
        (*m_data_->mAttrs)[name] = value.m_data_;
        return *this;
    }

    [[nodiscard]] BoxedValue getAttr(const std::string& name) const {
        std::shared_lock lock(m_mutex_);
        if (m_data_->mAttrs) {
            if (auto it = m_data_->mAttrs->find(name);
                it != m_data_->mAttrs->end()) {
                return BoxedValue(it->second);
            }
        }
        return {};  // Undefined BoxedValue
    }

    [[nodiscard]] bool hasAttr(const std::string& name) const {
        std::shared_lock lock(m_mutex_);
        return m_data_->mAttrs &&
               m_data_->mAttrs->find(name) != m_data_->mAttrs->end();
    }

    void removeAttr(const std::string& name) {
        std::unique_lock lock(m_mutex_);
        if (m_data_->mAttrs) {
            m_data_->mAttrs->erase(name);
        }
    }

    [[nodiscard]] std::vector<std::string> listAttrs() const {
        std::shared_lock lock(m_mutex_);
        std::vector<std::string> attrs;
        if (m_data_->mAttrs) {
            attrs.reserve(m_data_->mAttrs->size());
            for (const auto& entry : *m_data_->mAttrs) {
                attrs.push_back(entry.first);
            }
        }
        return attrs;
    }

    [[nodiscard]] bool isNull() const noexcept {
        std::shared_lock lock(m_mutex_);
        return !m_data_->mObj.has_value();
    }

    [[nodiscard]] void* getPtr() const noexcept {
        std::shared_lock lock(m_mutex_);
        return const_cast<void*>(m_data_->mConstDataPtr);
    }

    // Try casting the contained value to a specific type
    template <typename T>
    [[nodiscard]] std::optional<T> tryCast() const noexcept {
        std::shared_lock lock(m_mutex_);
        try {
            if constexpr (std::is_reference_v<T>) {
                if (m_data_->mObj.type() ==
                    typeid(
                        std::reference_wrapper<std::remove_reference_t<T>>)) {
                    return std::any_cast<std::reference_wrapper<
                        std::remove_reference_t<T>>>(m_data_->mObj)
                        .get();
                }
            }
            if (m_data_->mObj.type() == typeid(std::reference_wrapper<T>)) {
                return std::any_cast<std::reference_wrapper<T>>(m_data_->mObj)
                    .get();
            }
            if (isConst() || isReadonly()) {
                using constT = std::add_const_t<T>;
                return std::any_cast<constT>(m_data_->mObj);
            }
            return std::any_cast<T>(m_data_->mObj);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    // Check if the contained value can be cast to a specific type
    template <typename T>
    [[nodiscard]] bool canCast() const noexcept {
        std::shared_lock lock(m_mutex_);
        try {
            if constexpr (std::is_reference_v<T>) {
                return m_data_->mObj.type() ==
                       typeid(
                           std::reference_wrapper<std::remove_reference_t<T>>);
            } else {
                std::any_cast<T>(m_data_->mObj);
                return true;
            }
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }

    // Debug string representation
    [[nodiscard]] std::string debugString() const {
        std::ostringstream oss;
        oss << "BoxedValue<" << m_data_->mTypeInfo.name() << ">: ";
        std::shared_lock lock(m_mutex_);
        if (auto* intPtr = std::any_cast<int>(&m_data_->mObj)) {
            oss << *intPtr;
        } else if (auto* doublePtr = std::any_cast<double>(&m_data_->mObj)) {
            oss << *doublePtr;
        } else if (auto* strPtr = std::any_cast<std::string>(&m_data_->mObj)) {
            oss << *strPtr;
        } else {
            oss << "unknown type";
        }
        return oss.str();
    }
};

// Helper functions to create BoxedValue instances
template <typename T>
auto var(T&& t) -> BoxedValue {
    using DecayedType = std::decay_t<T>;
    constexpr bool IS_REF_WRAPPER =
        std::is_same_v<DecayedType,
                       std::reference_wrapper<std::remove_reference_t<T>>>;
    return BoxedValue(std::forward<T>(t), IS_REF_WRAPPER, false);
}

template <typename T>
auto constVar(const T& t) -> BoxedValue {
    using DecayedType = std::decay_t<T>;
    constexpr bool IS_REF_WRAPPER =
        std::is_same_v<DecayedType,
                       std::reference_wrapper<std::remove_reference_t<T>>>;
    return BoxedValue(std::cref(t), IS_REF_WRAPPER, true);
}

inline auto voidVar() -> BoxedValue { return {}; }

// Factory functions for creating BoxedValues
template <typename T>
auto makeBoxedValue(T&& t, bool is_return_value = false,
                    bool readonly = false) -> BoxedValue {
    if constexpr (std::is_reference_v<T>) {
        return BoxedValue(std::ref(t), is_return_value, readonly);
    } else {
        return BoxedValue(std::forward<T>(t), is_return_value, readonly);
    }
}

}  // namespace atom::meta

#endif  // ATOM_META_ANY_HPP
