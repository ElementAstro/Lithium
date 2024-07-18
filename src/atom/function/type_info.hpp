/*!
 * \file type_info.hpp
 * \brief Enhance TypeInfo for better type handling
 * \author Max Qian <lightapt.com>
 * \date 2023-04-05
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TypeInfo_HPP
#define ATOM_META_TypeInfo_HPP

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

/// \brief Compile time deduced information about a type
class TypeInfo {
public:
    /// \brief Construct a new Type Info object
    ATOM_CONSTEXPR TypeInfo(bool t_is_const, bool t_is_reference,
                            bool t_is_pointer, bool t_is_void,
                            bool t_is_arithmetic, bool t_is_array,
                            bool t_is_enum, bool t_is_class, bool t_is_function,
                            bool t_is_trivial, bool t_is_standard_layout,
                            bool t_is_pod, const std::type_info *t_ti,
                            const std::type_info *t_bare_ti) ATOM_NOEXCEPT
        : mTypeInfo_(t_ti),
          mBareTypeInfo_(t_bare_ti),
          mFlags_((static_cast<unsigned int>(t_is_const) << IS_CONST_FLAG) |
                  (static_cast<unsigned int>(t_is_reference)
                   << IS_REFERENCE_FLAG) |
                  (static_cast<unsigned int>(t_is_pointer) << IS_POINTER_FLAG) |
                  (static_cast<unsigned int>(t_is_void) << IS_VOID_FLAG) |
                  (static_cast<unsigned int>(t_is_arithmetic)
                   << IS_ARITHMETIC_FLAG)),
          mTypeProperties_(
              (static_cast<unsigned int>(t_is_array) << IS_ARRAY_FLAG) |
              (static_cast<unsigned int>(t_is_enum) << IS_ENUM_FLAG) |
              (static_cast<unsigned int>(t_is_class) << IS_CLASS_FLAG) |
              (static_cast<unsigned int>(t_is_function) << IS_FUNCTION_FLAG) |
              (static_cast<unsigned int>(t_is_trivial) << IS_TRIVIAL_FLAG) |
              (static_cast<unsigned int>(t_is_standard_layout)
               << IS_STANDARD_LAYOUT_FLAG) |
              (static_cast<unsigned int>(t_is_pod) << IS_POD_FLAG)) {}

    ATOM_CONSTEXPR TypeInfo() ATOM_NOEXCEPT = default;

    template <typename T>
    static auto fromType() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo(
            std::is_const_v<typename std::remove_reference<T>::type>, std::is_reference_v<T>, std::is_pointer_v<T>,
            std::is_void_v<T>, std::is_arithmetic_v<T>, std::is_array_v<T>,
            std::is_enum_v<T>, std::is_class_v<T>, std::is_function_v<T>,
            std::is_trivial_v<T>, std::is_standard_layout_v<T>,
            std::is_standard_layout_v<T> && std::is_trivial_v<T>, &typeid(T),
            &typeid(BareT));
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

    ATOM_CONSTEXPR auto operator!=(const std::type_info &ti) const
        ATOM_NOEXCEPT->bool {
        return !(*this == ti);
    }

    ATOM_CONSTEXPR auto operator==(const TypeInfo &ti) const
        ATOM_NOEXCEPT->bool {
        return ti.mTypeInfo_ == mTypeInfo_ && *ti.mTypeInfo_ == *mTypeInfo_ &&
               ti.mBareTypeInfo_ == mBareTypeInfo_ &&
               *ti.mBareTypeInfo_ == *mBareTypeInfo_ && ti.mFlags_ == mFlags_ &&
               ti.mTypeProperties_ == mTypeProperties_;
    }

    ATOM_CONSTEXPR auto operator==(const std::type_info &ti) const
        ATOM_NOEXCEPT->bool {
        return !isUndef() && (*mTypeInfo_) == ti;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto bareEqual(const TypeInfo &ti) const
        ATOM_NOEXCEPT -> bool {
        return ti.mBareTypeInfo_ == mBareTypeInfo_ ||
               *ti.mBareTypeInfo_ == *mBareTypeInfo_;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto bareEqualTypeInfo(
        const std::type_info &ti) const ATOM_NOEXCEPT -> bool {
        return !isUndef() && (*mBareTypeInfo_) == ti;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isConst() const ATOM_NOEXCEPT -> bool {
        return (mFlags_ & (1 << IS_CONST_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isReference() const ATOM_NOEXCEPT
        -> bool {
        return (mFlags_ & (1 << IS_REFERENCE_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isVoid() const ATOM_NOEXCEPT -> bool {
        return (mFlags_ & (1 << IS_VOID_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isArithmetic() const ATOM_NOEXCEPT
        -> bool {
        return (mFlags_ & (1 << IS_ARITHMETIC_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isArray() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_ARRAY_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isEnum() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_ENUM_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isClass() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_CLASS_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isFunction() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_FUNCTION_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isTrivial() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_TRIVIAL_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isStandardLayout() const ATOM_NOEXCEPT
        -> bool {
        return (mTypeProperties_ & (1 << IS_STANDARD_LAYOUT_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isPod() const ATOM_NOEXCEPT -> bool {
        return (mTypeProperties_ & (1 << IS_POD_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isPointer() const ATOM_NOEXCEPT -> bool {
        return (mFlags_ & (1 << IS_POINTER_FLAG)) != 0;
    }

    [[nodiscard]] ATOM_CONSTEXPR auto isUndef() const ATOM_NOEXCEPT -> bool {
        return (mFlags_ & (1 << IS_UNDEF_FLAG)) != 0;
    }

    [[nodiscard]] auto name() const ATOM_NOEXCEPT -> std::string {
        if (!isUndef()) {
            return DemangleHelper::demangle(mTypeInfo_->name());
        }
        return "undefined";
    }

    [[nodiscard]] auto bareName() const ATOM_NOEXCEPT -> std::string {
        if (!isUndef()) {
            return DemangleHelper::demangle(mBareTypeInfo_->name());
        }
        return "undefined";
    }

    [[nodiscard]] ATOM_CONSTEXPR auto bareTypeInfo() const ATOM_NOEXCEPT
        -> const std::type_info * {
        return mBareTypeInfo_;
    }

private:
    struct UnknownType {};

    const std::type_info *mTypeInfo_ = &typeid(UnknownType);
    const std::type_info *mBareTypeInfo_ = &typeid(UnknownType);
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_CONST_FLAG = 0;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_REFERENCE_FLAG = 1;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_POINTER_FLAG = 2;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_VOID_FLAG = 3;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_ARITHMETIC_FLAG = 4;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_UNDEF_FLAG = 5;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_ARRAY_FLAG = 6;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_ENUM_FLAG = 7;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_CLASS_FLAG = 8;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_FUNCTION_FLAG = 9;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_TRIVIAL_FLAG = 10;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_STANDARD_LAYOUT_FLAG = 11;
    ATOM_INLINE static ATOM_CONSTEXPR unsigned int IS_POD_FLAG = 12;
    unsigned int mFlags_ = (1 << IS_UNDEF_FLAG);
    unsigned int mTypeProperties_ = 0;
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
        using BareT = BareType<T>;
        return TypeInfo(std::is_const_v<T>, false, true, std::is_void_v<BareT>,
                        std::is_arithmetic_v<BareT>, std::is_array_v<T>,
                        std::is_enum_v<T>, std::is_class_v<T>,
                        std::is_function_v<T>, std::is_trivial_v<T>,
                        std::is_standard_layout_v<T>,
                        std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                        &typeid(std::shared_ptr<T>), &typeid(BareT));
    }
};

template <typename T>
struct GetTypeInfo<std::shared_ptr<T> &> : GetTypeInfo<std::shared_ptr<T>> {};

// Specialization for const std::shared_ptr<T>&
template <typename T>
struct GetTypeInfo<const std::shared_ptr<T> &> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo(std::is_const_v<T>, true, true, std::is_void_v<BareT>,
                        std::is_arithmetic_v<BareT>, std::is_array_v<T>,
                        std::is_enum_v<T>, std::is_class_v<T>,
                        std::is_function_v<T>, std::is_trivial_v<T>,
                        std::is_standard_layout_v<T>,
                        std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                        &typeid(const std::shared_ptr<T> &), &typeid(BareT));
    }
};

// Specialization for std::reference_wrapper<T>
template <typename T>
struct GetTypeInfo<std::reference_wrapper<T>> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo(
            std::is_const_v<T>,
            false,  // reference_wrapper is never const itself but T might be
            false, std::is_void_v<BareT>, std::is_arithmetic_v<BareT>,
            std::is_array_v<T>, std::is_enum_v<T>, std::is_class_v<T>,
            std::is_function_v<T>, std::is_trivial_v<T>,
            std::is_standard_layout_v<T>,
            std::is_standard_layout_v<T> && std::is_trivial_v<T>,
            &typeid(std::reference_wrapper<T>), &typeid(BareT));
    }
};

// Specialization for const std::reference_wrapper<T>&
template <typename T>
struct GetTypeInfo<const std::reference_wrapper<T> &> {
    ATOM_CONSTEXPR static auto get() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo(
            std::is_const_v<T>, true, false, std::is_void_v<BareT>,
            std::is_arithmetic_v<BareT>, std::is_array_v<T>, std::is_enum_v<T>,
            std::is_class_v<T>, std::is_function_v<T>, std::is_trivial_v<T>,
            std::is_standard_layout_v<T>,
            std::is_standard_layout_v<T> && std::is_trivial_v<T>,
            &typeid(const std::reference_wrapper<T> &), &typeid(BareT));
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
    std::ostream &os, const atom::meta::TypeInfo &TypeInfo) -> std::ostream & {
    return os << TypeInfo.name();
}

namespace std {
template <>
struct hash<atom::meta::TypeInfo> {
    auto operator()(const atom::meta::TypeInfo &TypeInfo) const
        ATOM_NOEXCEPT->std::size_t {
        if (TypeInfo.isUndef()) {
            return 0;
        }
        return std::hash<const std::type_info *>{}(TypeInfo.bareTypeInfo()) ^
               (std::hash<std::string>{}(TypeInfo.name()) << 2) ^
               (std::hash<std::string>{}(TypeInfo.bareName()) << 3);
    }
};
}  // namespace std
#endif
