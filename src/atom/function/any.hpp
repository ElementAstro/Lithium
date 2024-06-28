/*!
 * \file any.hpp
 * \brief BoxedValue
 * \author Max Qian <lightapt.com>
 * \date 2023-12-28
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ANY_HPP
#define ATOM_META_ANY_HPP

#include <any>
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
#include <variant>
#include <vector>

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
        Data(T&& obj, bool is_ref, bool return_value, bool readonly)
            : mObj(std::forward<T>(obj)),
              mTypeInfo(user_type<std::decay_t<T>>()),
              mAttrs(nullptr),
              mIsRef(is_ref),
              mReturnValue(return_value),
              mReadonly(readonly),
              mConstDataPtr((const void*)std::addressof(obj)) {}
    };

    std::shared_ptr<Data> m_data_;
    mutable std::shared_mutex m_mutex_;

public:
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<BoxedValue, std::decay_t<T>>>>
    explicit BoxedValue(T&& t, bool t_return_value = false,
                        bool readonly = false)
        : m_data_(std::make_shared<Data>(std::forward<T>(t),
                                         std::is_reference_v<T>, t_return_value,
                                         readonly)) {}

    BoxedValue()
        : m_data_(std::make_shared<Data>(VoidType{}, false, false, false)) {}

    BoxedValue(const BoxedValue& other) {
        std::shared_lock lock(other.m_mutex_);
        m_data_ = other.m_data_;
    }

    BoxedValue(BoxedValue&& other) noexcept {
        std::unique_lock lock(other.m_mutex_);
        m_data_ = std::move(other.m_data_);
    }

    auto operator=(const BoxedValue& other) -> BoxedValue& {
        if (this != &other) {
            std::unique_lock lock(m_mutex_);
            std::shared_lock otherLock(other.m_mutex_);
            m_data_ = other.m_data_;
        }
        return *this;
    }

    auto operator=(BoxedValue&& other) noexcept -> BoxedValue& {
        if (this != &other) {
            std::unique_lock lock(m_mutex_);
            std::unique_lock otherLock(other.m_mutex_);
            m_data_ = std::move(other.m_data_);
        }
        return *this;
    }

    void swap(BoxedValue& rhs) noexcept {
        if (this != &rhs) {
            std::unique_lock lock1(m_mutex_, std::defer_lock);
            std::unique_lock lock2(rhs.m_mutex_, std::defer_lock);
            std::lock(lock1, lock2);
            std::swap(m_data_, rhs.m_data_);
        }
    }

    auto isUndef() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mObj.type() == typeid(VoidType);
    }

    auto isConst() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo.is_const();
    }

    auto isType(const TypeInfo& ti) const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo == ti;
    }

    auto isRef() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mIsRef;
    }

    auto isReturnValue() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mReturnValue;
    }

    void resetReturnValue() noexcept {
        std::unique_lock lock(m_mutex_);
        m_data_->mReturnValue = false;
    }

    auto isReadonly() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mReadonly;
    }

    auto isConstDataPtr() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mConstDataPtr != nullptr;
    }

    auto get() const noexcept -> const std::any& {
        std::shared_lock lock(m_mutex_);
        return m_data_->mObj;
    }

    auto getTypeInfo() const noexcept -> const TypeInfo& {
        std::shared_lock lock(m_mutex_);
        return m_data_->mTypeInfo;
    }

    auto setAttr(const std::string& name,
                 const BoxedValue& value) -> BoxedValue& {
        std::unique_lock lock(m_mutex_);
        if (!m_data_->mAttrs) {
            m_data_->mAttrs = std::make_shared<
                std::map<std::string, std::shared_ptr<Data>>>();
        }
        (*m_data_->mAttrs)[name] = value.m_data_;
        return *this;
    }

    auto getAttr(const std::string& name) const -> BoxedValue {
        std::shared_lock lock(m_mutex_);
        if (m_data_->mAttrs) {
            auto it = m_data_->mAttrs->find(name);
            if (it != m_data_->mAttrs->end()) {
                return BoxedValue(it->second);
            }
        }
        return {};  // Return undefined BoxedValue
    }

    auto hasAttr(const std::string& name) const -> bool {
        std::shared_lock lock(m_mutex_);
        return m_data_->mAttrs &&
               (m_data_->mAttrs->find(name) != m_data_->mAttrs->end());
    }

    void removeAttr(const std::string& name) {
        std::unique_lock lock(m_mutex_);
        if (m_data_->mAttrs) {
            m_data_->mAttrs->erase(name);
        }
    }

    auto listAttrs() const -> std::vector<std::string> {
        std::shared_lock lock(m_mutex_);
        std::vector<std::string> attrs;
        if (m_data_->mAttrs) {
            for (const auto& entry : *m_data_->mAttrs) {
                attrs.push_back(entry.first);
            }
        }
        return attrs;
    }

    auto isNull() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        if (m_data_->mObj.has_value()) {
            try {
                return std::any_cast<std::nullptr_t>(m_data_->mObj) != nullptr;
            } catch (const std::bad_any_cast&) {
                return false;
            }
        }
        return true;
    }

    auto getPtr() const noexcept -> void* {
        std::shared_lock lock(m_mutex_);
        return const_cast<void*>(m_data_->mConstDataPtr);
    }

    template <typename T>
    auto tryCast() const noexcept -> std::optional<T> {
        std::shared_lock lock(m_mutex_);
        try {
            if constexpr (std::is_reference_v<T>) {
                if (m_data_->mObj.type() ==
                    typeid(
                        std::reference_wrapper<std::remove_reference_t<T>>)) {
                    auto& ref = std::any_cast<
                        std::reference_wrapper<std::remove_reference_t<T>>>(
                        m_data_->mObj);
                    return ref.get();
                }
            }
            return std::any_cast<T>(m_data_->mObj);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    template <typename T>
    auto canCast() const noexcept -> bool {
        std::shared_lock lock(m_mutex_);
        try {
            if constexpr (std::is_reference_v<T>) {
                if (m_data_->mObj.type() == typeid(std::reference_wrapper<T>)) {
                    return true;
                }
            } else {
                std::any_cast<T>(m_data_->mObj);
                return true;
            }
        } catch (const std::bad_any_cast&) {
            return false;
        }
        return false;
    }

    /// Debug string representation of the contained object
    auto debugString() const -> std::string {
        std::ostringstream oss;
        oss << "BoxedValue<" << m_data_->mTypeInfo.name() << ">: ";
        if (m_data_->mObj.type() == typeid(int)) {
            oss << std::any_cast<int>(m_data_->mObj);
        } else if (m_data_->mObj.type() == typeid(double)) {
            oss << std::any_cast<double>(m_data_->mObj);
        } else if (m_data_->mObj.type() == typeid(std::string)) {
            oss << std::any_cast<std::string>(m_data_->mObj);
        } else if (m_data_->mObj.type() == typeid(bool)) {
            oss << std::any_cast<bool>(m_data_->mObj);
        } else if (m_data_->mObj.type() == typeid(std::vector<int>)) {
            oss << "vector<int>";
        } else if (m_data_->mObj.type() == typeid(std::vector<double>)) {
            oss << "vector<double>";
        } else if (m_data_->mObj.type() == typeid(std::vector<std::string>)) {
            oss << "vector<string>";
        } else if (m_data_->mObj.type() == typeid(std::vector<bool>)) {
            oss << "vector<bool>";
        } else {
            oss << "unknown type";
        }
        return oss.str();
    }

    /// Visitor pattern implementation
    template <typename Visitor>
    auto visit(Visitor&& visitor) {
        std::shared_lock lock(m_mutex_);
        return std::visit(std::forward<Visitor>(visitor), m_data_->mObj);
    }
};

// Helper function to create a non-constant BoxedValue
template <typename T>
auto var(T&& t) -> BoxedValue {
    using DecayedType = std::decay_t<T>;
    constexpr bool IS_REF_WRAPPER =
        std::is_same_v<DecayedType,
                       std::reference_wrapper<std::remove_reference_t<T>>>;
    return BoxedValue(std::forward<T>(t), IS_REF_WRAPPER, false);
}

// Helper function to create a constant BoxedValue
template <typename T>
auto constVar(const T& t) -> BoxedValue {
    using DecayedType = std::decay_t<T>;
    constexpr bool IS_REF_WRAPPER =
        std::is_same_v<DecayedType,
                       std::reference_wrapper<std::remove_reference_t<T>>>;
    return BoxedValue(std::cref(t), IS_REF_WRAPPER, true);
}

// Helper function to create a void BoxedValue
inline auto voidVar() -> BoxedValue { return {}; }

// Factory functions
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
