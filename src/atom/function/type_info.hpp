/*!
 * \file type_info.hpp
 * \brief Enhance type_info for better type handling
 * \author Max Qian <lightapt.com>
 * \date 2023-04-05
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TYPE_INFO_HPP
#define ATOM_META_TYPE_INFO_HPP

#include <cstdlib>
#include <functional>  // For std::hash
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "abi.hpp"

namespace atom::meta {
// Helper to remove cv-qualifiers, references, and pointers
template <typename T>
using Bare_Type =
    std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

/// \brief Compile time deduced information about a type
class Type_Info {
public:
    /// \brief Construct a new Type Info object
    constexpr Type_Info(bool t_is_const, bool t_is_reference, bool t_is_pointer,
                        bool t_is_void, bool t_is_arithmetic, bool t_is_array,
                        bool t_is_enum, bool t_is_class, bool t_is_function,
                        bool t_is_trivial, bool t_is_standard_layout,
                        bool t_is_pod, const std::type_info *t_ti,
                        const std::type_info *t_bare_ti) noexcept
        : m_type_info(t_ti),
          m_bare_type_info(t_bare_ti),
          m_flags(
              (static_cast<unsigned int>(t_is_const) << is_const_flag) |
              (static_cast<unsigned int>(t_is_reference) << is_reference_flag) |
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

    constexpr Type_Info() noexcept = default;

    template <typename T>
    static Type_Info from_type() noexcept {
        using BareT = Bare_Type<T>;
        return Type_Info(std::is_const_v<std::remove_reference_t<T>>,
                         std::is_reference_v<T>, std::is_pointer_v<T>,
                         std::is_void_v<T>, std::is_arithmetic_v<T>,
                         std::is_array_v<T>, std::is_enum_v<T>,
                         std::is_class_v<T>, std::is_function_v<T>,
                         std::is_trivial_v<T>, std::is_standard_layout_v<T>,
                         std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                         &typeid(T), &typeid(BareT));
    }

    template <typename T>
    static Type_Info from_instance(const T &) noexcept {
        return from_type<T>();
    }

    bool operator<(const Type_Info &ti) const noexcept {
        return m_type_info->before(*ti.m_type_info);
    }

    constexpr bool operator!=(const Type_Info &ti) const noexcept {
        return !(*this == ti);
    }

    constexpr bool operator!=(const std::type_info &ti) const noexcept {
        return !(*this == ti);
    }

    constexpr bool operator==(const Type_Info &ti) const noexcept {
        return ti.m_type_info == m_type_info &&
               *ti.m_type_info == *m_type_info &&
               ti.m_bare_type_info == m_bare_type_info &&
               *ti.m_bare_type_info == *m_bare_type_info &&
               ti.m_flags == m_flags &&
               ti.m_type_properties == m_type_properties;
    }

    constexpr bool operator==(const std::type_info &ti) const noexcept {
        return !is_undef() && (*m_type_info) == ti;
    }

    constexpr bool bare_equal(const Type_Info &ti) const noexcept {
        return ti.m_bare_type_info == m_bare_type_info ||
               *ti.m_bare_type_info == *m_bare_type_info;
    }

    constexpr bool bare_equal_type_info(
        const std::type_info &ti) const noexcept {
        return !is_undef() && (*m_bare_type_info) == ti;
    }

    constexpr bool is_const() const noexcept {
        return (m_flags & (1 << is_const_flag)) != 0;
    }

    constexpr bool is_reference() const noexcept {
        return (m_flags & (1 << is_reference_flag)) != 0;
    }

    constexpr bool is_void() const noexcept {
        return (m_flags & (1 << is_void_flag)) != 0;
    }

    constexpr bool is_arithmetic() const noexcept {
        return (m_flags & (1 << is_arithmetic_flag)) != 0;
    }

    constexpr bool is_array() const noexcept {
        return (m_type_properties & (1 << is_array_flag)) != 0;
    }

    constexpr bool is_enum() const noexcept {
        return (m_type_properties & (1 << is_enum_flag)) != 0;
    }

    constexpr bool is_class() const noexcept {
        return (m_type_properties & (1 << is_class_flag)) != 0;
    }

    constexpr bool is_function() const noexcept {
        return (m_type_properties & (1 << is_function_flag)) != 0;
    }

    constexpr bool is_trivial() const noexcept {
        return (m_type_properties & (1 << is_trivial_flag)) != 0;
    }

    constexpr bool is_standard_layout() const noexcept {
        return (m_type_properties & (1 << is_standard_layout_flag)) != 0;
    }

    constexpr bool is_pod() const noexcept {
        return (m_type_properties & (1 << is_pod_flag)) != 0;
    }

    constexpr bool is_pointer() const noexcept {
        return (m_flags & (1 << is_pointer_flag)) != 0;
    }

    constexpr bool is_undef() const noexcept {
        return (m_flags & (1 << is_undef_flag)) != 0;
    }

    std::string name() const noexcept {
        if (!is_undef()) {
            return DemangleHelper::Demangle(m_type_info->name());
        } else {
            return "undefined";
        }
    }

    std::string bare_name() const noexcept {
        if (!is_undef()) {
            return DemangleHelper::Demangle(m_bare_type_info->name());
        } else {
            return "undefined";
        }
    }

    constexpr const std::type_info *bare_type_info() const noexcept {
        return m_bare_type_info;
    }

    struct Unknown_Type {};

    const std::type_info *m_type_info = &typeid(Unknown_Type);
    const std::type_info *m_bare_type_info = &typeid(Unknown_Type);
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
struct Get_Type_Info {
    constexpr static Type_Info get() noexcept {
        return Type_Info::from_type<T>();
    }
};

// Specialization for std::shared_ptr<T>
template <typename T>
struct Get_Type_Info<std::shared_ptr<T>> {
    constexpr static Type_Info get() noexcept {
        using BareT = Bare_Type<T>;
        return Type_Info(std::is_const_v<T>, false, true, std::is_void_v<BareT>,
                         std::is_arithmetic_v<BareT>, std::is_array_v<T>,
                         std::is_enum_v<T>, std::is_class_v<T>,
                         std::is_function_v<T>, std::is_trivial_v<T>,
                         std::is_standard_layout_v<T>,
                         std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                         &typeid(std::shared_ptr<T>), &typeid(BareT));
    }
};

template <typename T>
struct Get_Type_Info<std::shared_ptr<T> &> : Get_Type_Info<std::shared_ptr<T>> {
};

// Specialization for const std::shared_ptr<T>&
template <typename T>
struct Get_Type_Info<const std::shared_ptr<T> &> {
    constexpr static Type_Info get() noexcept {
        using BareT = Bare_Type<T>;
        return Type_Info(std::is_const_v<T>, true, true, std::is_void_v<BareT>,
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
struct Get_Type_Info<std::reference_wrapper<T>> {
    constexpr static Type_Info get() noexcept {
        using BareT = Bare_Type<T>;
        return Type_Info(
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
struct Get_Type_Info<const std::reference_wrapper<T> &> {
    constexpr static Type_Info get() noexcept {
        using BareT = Bare_Type<T>;
        return Type_Info(
            std::is_const_v<T>, true, false, std::is_void_v<BareT>,
            std::is_arithmetic_v<BareT>, std::is_array_v<T>, std::is_enum_v<T>,
            std::is_class_v<T>, std::is_function_v<T>, std::is_trivial_v<T>,
            std::is_standard_layout_v<T>,
            std::is_standard_layout_v<T> && std::is_trivial_v<T>,
            &typeid(const std::reference_wrapper<T> &), &typeid(BareT));
    }
};

template <typename T>
constexpr Type_Info user_type(const T & /*t*/) noexcept {
    return Get_Type_Info<T>::get();
}

template <typename T>
constexpr Type_Info user_type() noexcept {
    return Get_Type_Info<T>::get();
}

namespace detail {
inline std::unordered_map<std::string, Type_Info> &get_type_registry() {
    static std::unordered_map<std::string, Type_Info> type_registry;
    return type_registry;
}

template <typename T>
struct Type_Registrar {
    Type_Registrar(const std::string &type_name) {
        detail::get_type_registry()[type_name] = user_type<T>();
    }
};
}  // namespace detail

inline void register_type(const std::string &type_name, Type_Info type_info) {
    detail::get_type_registry()[type_name] = type_info;
}

template <typename T>
inline void register_type(const std::string &type_name) {
    detail::get_type_registry()[type_name] = user_type<T>();
}

inline std::optional<Type_Info> get_type_info(const std::string &type_name) {
    auto &registry = detail::get_type_registry();
    auto it = registry.find(type_name);
    if (it != registry.end()) {
        return it->second;
    }
    return std::nullopt;
}
}  // namespace atom::meta

namespace std {

template <>
struct hash<atom::meta::Type_Info> {
    std::size_t operator()(
        const atom::meta::Type_Info &type_info) const noexcept {
        if (type_info.is_undef()) {
            return 0;
        }

        // Combine hashes of the type_info pointers and the flags
        std::size_t h1 =
            std::hash<const std::type_info *>{}(type_info.m_type_info);
        std::size_t h2 =
            std::hash<const std::type_info *>{}(type_info.m_bare_type_info);
        std::size_t h3 = std::hash<unsigned int>{}(type_info.m_flags);
        std::size_t h4 = std::hash<unsigned int>{}(type_info.m_type_properties);

        // Combine hashes using a method similar to boost's hash_combine
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

}  // namespace std
#endif
