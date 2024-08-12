/*!
 * \file type_info.hpp
 * \brief Enhance TypeInfo for better type handling
 * \author Max Qian <lightapt.com>
 * \date 2023-04-05
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TypeInfo_HPP
#define ATOM_META_TypeInfo_HPP

#include <bitset>
#include <cstdlib>
#include <functional>  // For std::hash
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "abi.hpp"
#include "atom/macro.hpp"
#include "god.hpp"

namespace atom::meta {
// Helper to remove cv-qualifiers, references, and pointers
template <typename T>
using BareType =
    std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

template <typename T>
struct is_shared_ptr : std::false_type {};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

// 检测是否为std::unique_ptr
template <typename T>
struct is_unique_ptr : std::false_type {};

template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

template <typename T>
constexpr bool is_pointer_like_v =
    std::is_pointer_v<T> || is_shared_ptr<T>::value || is_unique_ptr<T>::value;

template <typename T>
struct pointer_type {};

template <typename T>
struct pointer_type<T *> {
    using type = T;
};

template <typename T>
struct pointer_type<std::shared_ptr<T>> {
    using type = T;
};

template <typename T>
struct pointer_type<std::unique_ptr<T>> {
    using type = T;
};

template <typename T>
constexpr bool is_arithmetic_pointer_v =
    std::is_arithmetic_v<typename pointer_type<T>::type>;

/// \brief Compile time deduced information about a type
class TypeInfo {
public:
    using Flags = std::bitset<13>;  // Using bitset for flags
    /// \brief Construct a new Type Info object
    ATOM_CONSTEXPR TypeInfo(Flags flags, const std::type_info *t_ti,
                            const std::type_info *t_bare_ti) ATOM_NOEXCEPT
        : mTypeInfo_(t_ti),
          mBareTypeInfo_(t_bare_ti),
          mFlags_(flags) {}

    ATOM_CONSTEXPR TypeInfo() ATOM_NOEXCEPT = default;

    template <typename T>
    static auto fromType() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        Flags flags;
        flags.set(IS_CONST_FLAG, std::is_const_v<std::remove_reference_t<T>>);
        flags.set(IS_REFERENCE_FLAG, std::is_reference_v<T>);
        flags.set(IS_POINTER_FLAG, is_pointer_like_v<T>);
        flags.set(IS_VOID_FLAG, std::is_void_v<T>);
        if constexpr (is_pointer_like_v<T>) {
            flags.set(IS_ARITHMETIC_FLAG, is_arithmetic_pointer_v<T>);
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
    static auto fromInstance(const T &) ATOM_NOEXCEPT -> TypeInfo {
        return fromType<T>();
    }

    auto operator<(const TypeInfo &ti) const ATOM_NOEXCEPT->bool {
        return mTypeInfo_->before(*ti.mTypeInfo_);
    }

    ATOM_CONSTEXPR auto operator!=(const TypeInfo &ti) const
        ATOM_NOEXCEPT->bool {
        return !(*this == ti);
    }

    ATOM_CONSTEXPR auto operator==(const TypeInfo &ti) const
        ATOM_NOEXCEPT->bool {
        return ti.mTypeInfo_ == mTypeInfo_ && *ti.mTypeInfo_ == *mTypeInfo_ &&
               ti.mBareTypeInfo_ == mBareTypeInfo_ &&
               *ti.mBareTypeInfo_ == *mBareTypeInfo_ && ti.mFlags_ == mFlags_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto bareEqual(const TypeInfo &ti) const
        ATOM_NOEXCEPT -> bool {
        return ti.mBareTypeInfo_ == mBareTypeInfo_ ||
               *ti.mBareTypeInfo_ == *mBareTypeInfo_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto bareEqualTypeInfo(
        const std::type_info &ti) const ATOM_NOEXCEPT -> bool {
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

    ATOM_NODISCARD ATOM_CONSTEXPR auto isConst() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_CONST_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isReference() const ATOM_NOEXCEPT
        -> bool {
        return mFlags_.test(IS_REFERENCE_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isVoid() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_VOID_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isArithmetic() const ATOM_NOEXCEPT
        -> bool {
        return mFlags_.test(IS_ARITHMETIC_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isArray() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_ARRAY_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isEnum() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_ENUM_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isClass() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_CLASS_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isFunction() const ATOM_NOEXCEPT
        -> bool {
        return mFlags_.test(IS_FUNCTION_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isTrivial() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_TRIVIAL_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isStandardLayout() const ATOM_NOEXCEPT
        -> bool {
        return mFlags_.test(IS_STANDARD_LAYOUT_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isPod() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_POD_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isPointer() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_POINTER_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto isUndef() const ATOM_NOEXCEPT -> bool {
        return mFlags_.test(IS_UNDEF_FLAG);
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto bareTypeInfo() const ATOM_NOEXCEPT
        -> const std::type_info * {
        return mBareTypeInfo_;
    }

private:
    const std::type_info *mTypeInfo_ = &typeid(void);  // Adjusted for clarity
    const std::type_info *mBareTypeInfo_ = &typeid(void);
    Flags mFlags_ = Flags().set(IS_UNDEF_FLAG);  // Default to undefined

    // Preserve flag indices
    static constexpr unsigned int IS_CONST_FLAG = 0;
    static constexpr unsigned int IS_REFERENCE_FLAG = 1;
    static constexpr unsigned int IS_POINTER_FLAG = 2;
    static constexpr unsigned int IS_VOID_FLAG = 3;
    static constexpr unsigned int IS_ARITHMETIC_FLAG = 4;
    static constexpr unsigned int IS_UNDEF_FLAG = 5;
    static constexpr unsigned int IS_ARRAY_FLAG = 6;
    static constexpr unsigned int IS_ENUM_FLAG = 7;
    static constexpr unsigned int IS_CLASS_FLAG = 8;
    static constexpr unsigned int IS_FUNCTION_FLAG = 9;
    static constexpr unsigned int IS_TRIVIAL_FLAG = 10;
    static constexpr unsigned int IS_STANDARD_LAYOUT_FLAG = 11;
    static constexpr unsigned int IS_POD_FLAG = 12;
};

template <typename T>
struct GetTypeInfo {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<T>();
    }
};

// Specialization for std::shared_ptr<T>
template <typename T>
struct GetTypeInfo<std::shared_ptr<T>> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<std::shared_ptr<T>>();
    }
};

template <typename T>
struct GetTypeInfo<std::shared_ptr<T> &> : GetTypeInfo<std::shared_ptr<T>> {};

// Specialization for const std::shared_ptr<T>&
template <typename T>
struct GetTypeInfo<const std::shared_ptr<T> &> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<T>();
    }
};

// Specialization for const std::reference_wrapper<T>&
template <typename T>
struct GetTypeInfo<const std::reference_wrapper<T> &> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo::fromType<BareT>();
    }
};

template <typename T>
ATOM_CONSTEXPR auto userType(const T & /*t*/) ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

template <typename T>
ATOM_CONSTEXPR auto userType() ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

namespace detail {
ATOM_INLINE auto getTypeRegistry()
    -> std::unordered_map<std::string, TypeInfo> & {
    static std::unordered_map<std::string, TypeInfo> typeRegistry;
    return typeRegistry;
}

template <typename T>
struct TypeRegistrar {
    explicit TypeRegistrar(const std::string &type_name) {
        detail::getTypeRegistry()[type_name] = userType<T>();
    }
};
}  // namespace detail

ATOM_INLINE void registerType(const std::string &type_name, TypeInfo TypeInfo) {
    detail::getTypeRegistry()[type_name] = TypeInfo;
}

template <typename T>
ATOM_INLINE void registerType(const std::string &type_name) {
    detail::getTypeRegistry()[type_name] = userType<T>();
}

ATOM_INLINE auto getTypeInfo(const std::string &type_name)
    -> std::optional<TypeInfo> {
    auto &registry = detail::getTypeRegistry();
    auto findIt = registry.find(type_name);
    if (findIt != registry.end()) {
        return findIt->second;
    }
    return std::nullopt;
}
}  // namespace atom::meta

ATOM_INLINE auto operator<<(
    std::ostream &oss, const atom::meta::TypeInfo &typeInfo) -> std::ostream & {
    return oss << typeInfo.name();
}

namespace std {
template <>
struct hash<atom::meta::TypeInfo> {
    auto operator()(const atom::meta::TypeInfo &typeInfo) const
        ATOM_NOEXCEPT->std::size_t {
        if (typeInfo.isUndef()) {
            return 0;
        }
        return std::hash<const std::type_info *>{}(typeInfo.bareTypeInfo()) ^
               (std::hash<std::string>{}(typeInfo.name()) << 2) ^
               (std::hash<std::string>{}(typeInfo.bareName()) << 3);
    }
};
}  // namespace std
#endif
