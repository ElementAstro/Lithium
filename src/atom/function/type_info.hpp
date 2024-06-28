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
#include <string>
#include <type_traits>
#include <typeinfo>

#include "abi.hpp"
#include "god.hpp"
#include "atom/macro.hpp"

namespace atom::meta {
// Helper to remove cv-qualifiers, references, and pointers
template <typename T>
using BareType =
    std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

/// \brief Compile time deduced information about a type
class TypeInfo {
public:
    /// \brief Construct a new Type Info object
    constexpr TypeInfo(bool t_is_const, bool t_is_reference, bool t_is_pointer,
                       bool t_is_void, bool t_is_arithmetic, bool t_is_array,
                       bool t_is_enum, bool t_is_class, bool t_is_function,
                       bool t_is_trivial, bool t_is_standard_layout,
                       bool t_is_pod, const std::type_info *t_ti,
                       const std::type_info *t_bare_ti) ATOM_NOEXCEPT
        : m_TypeInfo(t_ti),
          m_bare_TypeInfo(t_bare_ti),
          m_flags((static_cast<unsigned int>(t_is_const) << is_const_flag) |
                  (static_cast<unsigned int>(t_is_reference)
                   << is_reference_flag) |
                  (static_cast<unsigned int>(t_is_pointer) << is_pointer_flag) |
                  (static_cast<unsigned int>(t_is_void) << is_void_flag) |
                  (static_cast<unsigned int>(t_is_arithmetic)
                   << is_arithmetic_flag)),
          m_type_properties(
              (static_cast<unsigned int>(t_is_array) << is_array_flag) |
              (static_cast<unsigned int>(t_is_enum) << is_enum_flag) |
              (static_cast<unsigned int>(t_is_class) << is_class_flag) |
              (static_cast<unsigned int>(t_is_function) << is_function_flag) |
              (static_cast<unsigned int>(t_is_trivial) << is_trivial_flag) |
              (static_cast<unsigned int>(t_is_standard_layout)
               << is_standard_layout_flag) |
              (static_cast<unsigned int>(t_is_pod) << is_pod_flag)) {}

    constexpr TypeInfo() ATOM_NOEXCEPT = default;

    template <typename T>
    static auto fromType() ATOM_NOEXCEPT -> TypeInfo {
        using BareT = BareType<T>;
        return TypeInfo(isConst<std::remove_reference_t<T>>(),
                        isRef<T>(), isPointer<T>(),
                        std::is_void_v<T>, std::is_arithmetic_v<T>,
                        std::is_array_v<T>, std::is_enum_v<T>,
                        std::is_class_v<T>, std::is_function_v<T>,
                        std::is_trivial_v<T>, std::is_standard_layout_v<T>,
                        std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                        &typeid(T), &typeid(BareT));
    }

    template <typename T>
    static auto fromInstance(const T &) ATOM_NOEXCEPT -> TypeInfo {
        return from_type<T>();
    }

    auto operator<(const TypeInfo &ti) const ATOM_NOEXCEPT->bool {
        return m_TypeInfo->before(*ti.m_TypeInfo);
    }

    constexpr auto operator!=(const TypeInfo &ti) const ATOM_NOEXCEPT->bool {
        return !(*this == ti);
    }

    constexpr auto operator!=(const std::type_info &ti) const
        ATOM_NOEXCEPT->bool {
        return !(*this == ti);
    }

    constexpr auto operator==(const TypeInfo &ti) const ATOM_NOEXCEPT->bool {
        return ti.m_TypeInfo == m_TypeInfo &&
               *ti.m_TypeInfo == *m_TypeInfo &&
               ti.m_bare_TypeInfo == m_bare_TypeInfo &&
               *ti.m_bare_TypeInfo == *m_bare_TypeInfo &&
               ti.m_flags == m_flags &&
               ti.m_type_properties == m_type_properties;
    }

    constexpr auto operator==(const std::type_info &ti) const
        ATOM_NOEXCEPT->bool {
        return !isUndef() && (*m_TypeInfo) == ti;
    }

    [[nodiscard]] constexpr auto bareEqual(const TypeInfo &ti) const
        ATOM_NOEXCEPT -> bool {
        return ti.m_bare_TypeInfo == m_bare_TypeInfo ||
               *ti.m_bare_TypeInfo == *m_bare_TypeInfo;
    }

    [[nodiscard]] constexpr auto bareEqualTypeInfo(
        const std::type_info &ti) const ATOM_NOEXCEPT -> bool {
        return !isUndef() && (*m_bare_TypeInfo) == ti;
    }

    [[nodiscard]] constexpr auto isConst() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_const_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isReference() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_reference_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isVoid() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_void_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isArithmetic() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_arithmetic_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isArray() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_array_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isEnum() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_enum_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isClass() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_class_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isFunction() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_function_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isTrivial() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_trivial_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isStandardLayout() const ATOM_NOEXCEPT
        -> bool {
        return (m_type_properties & (1 << is_standard_layout_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isPod() const ATOM_NOEXCEPT -> bool {
        return (m_type_properties & (1 << is_pod_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isPointer() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_pointer_flag)) != 0;
    }

    [[nodiscard]] constexpr auto isUndef() const ATOM_NOEXCEPT -> bool {
        return (m_flags & (1 << is_undef_flag)) != 0;
    }

    [[nodiscard]] auto name() const ATOM_NOEXCEPT -> std::string {
        if (!isUndef()) {
            return DemangleHelper::demangle(m_TypeInfo->name());
        }
        return "undefined";
    }

    [[nodiscard]] auto bareName() const ATOM_NOEXCEPT -> std::string {
        if (!isUndef()) {
            return DemangleHelper::demangle(m_bare_TypeInfo->name());
        }
        return "undefined";
    }

    [[nodiscard]] constexpr auto bareTypeInfo() const ATOM_NOEXCEPT
        -> const std::type_info * {
        return m_bare_TypeInfo;
    }

    struct UnknownType {};

    const std::type_info *m_TypeInfo = &typeid(UnknownType);
    const std::type_info *m_bare_TypeInfo = &typeid(UnknownType);
    inline static constexpr unsigned int is_const_flag = 0;
    inline static constexpr unsigned int is_reference_flag = 1;
    inline static constexpr unsigned int is_pointer_flag = 2;
    inline static constexpr unsigned int is_void_flag = 3;
    inline static constexpr unsigned int is_arithmetic_flag = 4;
    inline static constexpr unsigned int is_undef_flag = 5;
    inline static constexpr unsigned int is_array_flag = 6;
    inline static constexpr unsigned int is_enum_flag = 7;
    inline static constexpr unsigned int is_class_flag = 8;
    inline static constexpr unsigned int is_function_flag = 9;
    inline static constexpr unsigned int is_trivial_flag = 10;
    inline static constexpr unsigned int is_standard_layout_flag = 11;
    inline static constexpr unsigned int is_pod_flag = 12;
    unsigned int m_flags = (1 << is_undef_flag);
    unsigned int m_type_properties = 0;
};

template <typename T>
struct GetTypeInfo {
    constexpr static auto get() ATOM_NOEXCEPT -> TypeInfo {
        return TypeInfo::fromType<T>();
    }
};

// Specialization for std::shared_ptr<T>
template <typename T>
struct GetTypeInfo<std::shared_ptr<T>> {
    constexpr static auto get() ATOM_NOEXCEPT -> TypeInfo {
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
    constexpr static auto get() ATOM_NOEXCEPT -> TypeInfo {
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
    constexpr static auto get() ATOM_NOEXCEPT -> TypeInfo {
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
    constexpr static auto get() ATOM_NOEXCEPT -> TypeInfo {
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
constexpr auto userType(const T & /*t*/) ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

template <typename T>
constexpr auto userType() ATOM_NOEXCEPT -> TypeInfo {
    return GetTypeInfo<T>::get();
}

namespace detail {
inline auto getTypeRegistry() -> std::unordered_map<std::string, TypeInfo> & {
    static std::unordered_map<std::string, TypeInfo> typeRegistry;
    return typeRegistry;
}

template <typename T>
struct TypeRegistrar {
    explicit TypeRegistrar(const std::string &type_name) {
        detail::getTypeRegistry()[type_name] = user_type<T>();
    }
};
}  // namespace detail

inline void registerType(const std::string &type_name, TypeInfo TypeInfo) {
    detail::getTypeRegistry()[type_name] = TypeInfo;
}

template <typename T>
inline void registerType(const std::string &type_name) {
    detail::getTypeRegistry()[type_name] = user_type<T>();
}

inline auto getTypeInfo(const std::string &type_name)
    -> std::optional<TypeInfo> {
    auto &registry = detail::getTypeRegistry();
    auto findIt = registry.find(type_name);
    if (findIt != registry.end()) {
        return findIt->second;
    }
    return std::nullopt;
}
}  // namespace atom::meta

namespace std {

template <>
struct hash<atom::meta::TypeInfo> {
    auto operator()(const atom::meta::TypeInfo &TypeInfo) const
        ATOM_NOEXCEPT->std::size_t {
        if (TypeInfo.isUndef()) {
            return 0;
        }

        // Combine hashes of the TypeInfo pointers and the flags
        std::size_t h1 =
            std::hash<const std::type_info *>{}(TypeInfo.m_TypeInfo);
        std::size_t h2 =
            std::hash<const std::type_info *>{}(TypeInfo.m_bare_TypeInfo);
        std::size_t h3 = std::hash<unsigned int>{}(TypeInfo.m_flags);
        std::size_t h4 = std::hash<unsigned int>{}(TypeInfo.m_type_properties);

        // Combine hashes using a method similar to boost's hash_combine
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

}  // namespace std
#endif
