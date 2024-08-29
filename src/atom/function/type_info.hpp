/*!
 * \file type_info.hpp
 * \brief Enhance TypeInfo for better type handling
 * \author Max Qian <lightapt.com>
 * \date 2023-04-05
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TYPEINFO_HPP
#define ATOM_META_TYPEINFO_HPP

#include <bitset>
#include <cstdlib>
#include <functional>  // For std::hash
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

#include "abi.hpp"
#include "atom/macro.hpp"

namespace atom::meta {

// Helper to remove cv-qualifiers, references, and pointers
template <typename T>
using BareType =
    std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

template <typename T>
concept PointerLike = std::is_pointer_v<T> || requires {
    typename std::enable_if_t<
        std::is_same_v<std::remove_cvref_t<T>,
                       std::shared_ptr<typename T::element_type>> ||
        std::is_same_v<std::remove_cvref_t<T>,
                       std::unique_ptr<typename T::element_type>> ||
        std::is_same_v<std::remove_cvref_t<T>,
                       std::weak_ptr<typename T::element_type>>>;
};

template <typename T>
concept ArithmeticPointer =
    PointerLike<T> &&
    std::is_arithmetic_v<
        typename std::pointer_traits<std::remove_cvref_t<T>>::element_type>;

/// \brief Compile time deduced information about a type
class TypeInfo {
public:
    using Flags = std::bitset<13>;  // Using bitset for flags

    /// \brief Construct a new Type Info object
    consteval TypeInfo(Flags flags, const std::type_info* t_ti,
                       const std::type_info* t_bare_ti) ATOM_NOEXCEPT
        : mTypeInfo_(t_ti),
          mBareTypeInfo_(t_bare_ti),
          mFlags_(flags) {}

    consteval TypeInfo() ATOM_NOEXCEPT = default;

    template <typename T>
    static consteval auto fromType() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        Flags flags;
        flags.set(IS_CONST_FLAG, std::is_const_v<std::remove_reference_t<T>>);
        flags.set(IS_REFERENCE_FLAG, std::is_reference_v<T>);
        flags.set(IS_POINTER_FLAG, PointerLike<T>);
        flags.set(IS_VOID_FLAG, std::is_void_v<T>);
        if ATOM_CONSTEXPR (PointerLike<T>) {
            flags.set(IS_ARITHMETIC_FLAG, ArithmeticPointer<T>);
        } else {
            flags.set(IS_ARITHMETIC_FLAG, std::is_arithmetic_v<T>);
        }
        flags.set(IS_ARRAY_FLAG, std::is_array_v<T>);
        flags.set(IS_ENUM_FLAG, std::is_enum_v<T>);
        flags.set(IS_CLASS_FLAG, std::is_class_v<T>);
        flags.set(IS_FUNCTION_FLAG, std::is_function_v<T>);
        flags.set(IS_TRIVIAL_FLAG, std::is_trivial_v<T>);
        flags.set(IS_STANDARD_LAYOUT_FLAG, std::is_standard_layout_v<T>);
        flags.set(IS_POD_FLAG, flags.test(IS_TRIVIAL_FLAG) &&
                                   flags.test(IS_STANDARD_LAYOUT_FLAG));

        return {flags, &typeid(T), &typeid(BareT)};
    }

    template <typename T>
    static consteval auto fromInstance(const T&) ATOM_NOEXCEPT -> TypeInfo {
        return fromType<T>();
    }

    auto operator<(const TypeInfo& ti) const ATOM_NOEXCEPT->bool {
        return mTypeInfo_->before(*ti.mTypeInfo_);
    }

    ATOM_CONSTEXPR auto operator!=(const TypeInfo& ti) const
        ATOM_NOEXCEPT->bool {
        return !(*this == ti);
    }

    ATOM_CONSTEXPR auto operator==(const TypeInfo& ti) const
        ATOM_NOEXCEPT->bool {
        return ti.mTypeInfo_ == mTypeInfo_ && *ti.mTypeInfo_ == *mTypeInfo_ &&
               ti.mBareTypeInfo_ == mBareTypeInfo_ &&
               *ti.mBareTypeInfo_ == *mBareTypeInfo_ && ti.mFlags_ == mFlags_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto bareEqual(const TypeInfo& ti) const
        ATOM_NOEXCEPT -> bool {
        return ti.mBareTypeInfo_ == mBareTypeInfo_ ||
               *ti.mBareTypeInfo_ == *mBareTypeInfo_;
    }

    ATOM_NODISCARD auto bareEqualTypeInfo(const std::type_info& ti) const
        ATOM_NOEXCEPT -> bool {
        return !isUndef() && (*mBareTypeInfo_) == ti;
    }

    ATOM_NODISCARD auto name() const ATOM_NOEXCEPT -> std::string {
        return !isUndef() ? DemangleHelper::demangle(mTypeInfo_->name())
                          : "undefined";
    }

    ATOM_NODISCARD auto bareName() const ATOM_NOEXCEPT -> std::string {
        return !isUndef() ? DemangleHelper::demangle(mBareTypeInfo_->name())
                          : "undefined";
    }

    ATOM_NODISCARD auto isConst() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_CONST_FLAG);
    }

    ATOM_NODISCARD auto isReference() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_REFERENCE_FLAG);
    }

    ATOM_NODISCARD auto isVoid() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_VOID_FLAG);
    }

    ATOM_NODISCARD auto isArithmetic() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_ARITHMETIC_FLAG);
    }

    ATOM_NODISCARD auto isArray() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_ARRAY_FLAG);
    }

    ATOM_NODISCARD auto isEnum() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_ENUM_FLAG);
    }

    ATOM_NODISCARD auto isClass() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_CLASS_FLAG);
    }

    ATOM_NODISCARD auto isFunction() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_FUNCTION_FLAG);
    }

    ATOM_NODISCARD auto isTrivial() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_TRIVIAL_FLAG);
    }

    ATOM_NODISCARD auto isStandardLayout() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_STANDARD_LAYOUT_FLAG);
    }

    ATOM_NODISCARD auto isPod() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_POD_FLAG);
    }

    ATOM_NODISCARD auto isPointer() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_POINTER_FLAG);
    }

    ATOM_NODISCARD auto isUndef() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_UNDEF_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto bareTypeInfo() const ATOM_NOEXCEPT
        -> const std::type_info* {
        return mBareTypeInfo_;
    }

private:
    const std::type_info* mTypeInfo_ = &typeid(void);  // Adjusted for clarity
    const std::type_info* mBareTypeInfo_ = &typeid(void);
    Flags mFlags_ = Flags().set(IS_UNDEF_FLAG);  // Default to undefined

    // Preserve flag indices
    static ATOM_CONSTEXPR unsigned int IS_CONST_FLAG = 0;
    static ATOM_CONSTEXPR unsigned int IS_REFERENCE_FLAG = 1;
    static ATOM_CONSTEXPR unsigned int IS_POINTER_FLAG = 2;
    static ATOM_CONSTEXPR unsigned int IS_VOID_FLAG = 3;
    static ATOM_CONSTEXPR unsigned int IS_ARITHMETIC_FLAG = 4;
    static ATOM_CONSTEXPR unsigned int IS_UNDEF_FLAG = 5;
    static ATOM_CONSTEXPR unsigned int IS_ARRAY_FLAG = 6;
    static ATOM_CONSTEXPR unsigned int IS_ENUM_FLAG = 7;
    static ATOM_CONSTEXPR unsigned int IS_CLASS_FLAG = 8;
    static ATOM_CONSTEXPR unsigned int IS_FUNCTION_FLAG = 9;
    static ATOM_CONSTEXPR unsigned int IS_TRIVIAL_FLAG = 10;
    static ATOM_CONSTEXPR unsigned int IS_STANDARD_LAYOUT_FLAG = 11;
    static ATOM_CONSTEXPR unsigned int IS_POD_FLAG = 12;
};

template <typename T>
struct GetTypeInfo {
    consteval static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<T>();
    }
};

// Specialization for std::shared_ptr<T>
template <typename T>
struct GetTypeInfo<std::shared_ptr<T>> {
    consteval static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<std::shared_ptr<T>>();
    }
};

template <typename T>
struct GetTypeInfo<std::shared_ptr<T>&> : GetTypeInfo<std::shared_ptr<T>> {};

// Specialization for const std::shared_ptr<T>&
template <typename T>
struct GetTypeInfo<const std::shared_ptr<T>&> {
    consteval static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<T>();
    }
};

// Specialization for const std::reference_wrapper<T>&
template <typename T>
struct GetTypeInfo<const std::reference_wrapper<T>&> {
    consteval static auto get() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo::fromType<BareT>();
    }
};

template <typename T>
consteval auto userType(const T& /*t*/) ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

template <typename T>
consteval auto userType() ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

namespace detail {
ATOM_INLINE auto getTypeRegistry()
    -> std::unordered_map<std::string, TypeInfo>& {
    static std::unordered_map<std::string, TypeInfo> typeRegistry;
    return typeRegistry;
}

template <typename T>
struct TypeRegistrar {
    explicit TypeRegistrar(const std::string& type_name) {
        detail::getTypeRegistry()[type_name] = userType<T>();
    }
};
}  // namespace detail

ATOM_INLINE void registerType(const std::string& type_name, TypeInfo typeInfo) {
    detail::getTypeRegistry()[type_name] = std::move(typeInfo);
}

template <typename T>
ATOM_INLINE void registerType(const std::string& type_name) {
    detail::getTypeRegistry()[type_name] = userType<T>();
}

ATOM_INLINE auto getTypeInfo(const std::string& type_name)
    -> std::optional<TypeInfo> {
    auto& registry = detail::getTypeRegistry();
    auto findIt = registry.find(type_name);
    if (findIt != registry.end()) {
        return findIt->second;
    }
    return std::nullopt;
}
}  // namespace atom::meta

ATOM_INLINE auto operator<<(
    std::ostream& oss, const atom::meta::TypeInfo& typeInfo) -> std::ostream& {
    return oss << typeInfo.name();
}

namespace std {
template <>
struct hash<atom::meta::TypeInfo> {
    auto operator()(const atom::meta::TypeInfo& typeInfo) const
        ATOM_NOEXCEPT->std::size_t {
        if (typeInfo.isUndef()) {
            return 0;
        }
        return std::hash<const std::type_info*>{}(typeInfo.bareTypeInfo()) ^
               (std::hash<std::string>{}(typeInfo.name()) << 2) ^
               (std::hash<std::string>{}(typeInfo.bareName()) << 3);
    }
};
}  // namespace std

#endif  // ATOM_META_TYPEINFO_HPP
