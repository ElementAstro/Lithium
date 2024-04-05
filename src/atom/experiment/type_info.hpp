/*
 * type_info.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: Enhance type_info for better type handling

**************************************************/

#ifndef ATOM_EXPERIMENT_TYPE_INFO_HPP
#define ATOM_EXPERIMENT_TYPE_INFO_HPP

#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>

template <typename T>
struct Bare_Type {
    using type = typename std::remove_cv<typename std::remove_pointer<
        typename std::remove_reference<T>::type>::type>::type;
};

/// \brief Compile time deduced information about a type
class Type_Info {
public:
    /// \brief Construct a new Type Info object
    /// \param t_is_const Is the type const
    /// \param t_is_reference Is the type a reference
    /// \param t_is_pointer Is the type a pointer
    /// \param t_is_void Is the type void
    /// \param t_is_arithmetic Is the type arithmetic
    /// \param t_ti The type_info of the type
    /// \param t_bare_ti The type_info of the bare type
    constexpr Type_Info(const bool t_is_const, const bool t_is_reference,
                        const bool t_is_pointer, const bool t_is_void,
                        const bool t_is_arithmetic, const std::type_info *t_ti,
                        const std::type_info *t_bare_ti) noexcept
        : m_type_info(t_ti),
          m_bare_type_info(t_bare_ti),
          m_flags(
              (static_cast<unsigned int>(t_is_const) << is_const_flag) +
              (static_cast<unsigned int>(t_is_reference) << is_reference_flag) +
              (static_cast<unsigned int>(t_is_pointer) << is_pointer_flag) +
              (static_cast<unsigned int>(t_is_void) << is_void_flag) +
              (static_cast<unsigned int>(t_is_arithmetic)
               << is_arithmetic_flag)) {}

    /// \brief Construct a new Type Info object
    constexpr Type_Info() noexcept = default;

    /// \brief Compare two Type_Info objects
    /// \param ti The other Type_Info object
    /// \return true if the two Type_Info objects are equal
    /// \return false if the two Type_Info objects are not equal
    bool operator<(const Type_Info &ti) const noexcept {
        return m_type_info->before(*ti.m_type_info);
    }

    /// \brief Compare two Type_Info objects
    constexpr bool operator!=(const Type_Info &ti) const noexcept {
        return !(operator==(ti));
    }

    /// \brief Compare two Type_Info objects
    constexpr bool operator!=(const std::type_info &ti) const noexcept {
        return !(operator==(ti));
    }

    /// \brief Compare two Type_Info objects
    /// \param ti The other Type_Info object
    /// \return true if the two Type_Info objects are equal
    /// \return false if the two Type_Info objects are not equal
    constexpr bool operator==(const Type_Info &ti) const noexcept {
        return ti.m_type_info == m_type_info || *ti.m_type_info == *m_type_info;
    }

    /// \brief Compare two Type_Info objects
    /// \param ti The other Type_Info object
    /// \return true if the two Type_Info objects are equal
    /// \return false if the two Type_Info objects are not equal
    constexpr bool operator==(const std::type_info &ti) const noexcept {
        return !is_undef() && (*m_type_info) == ti;
    }

    /// \brief Compare two Type_Info objects
    /// \param ti The other Type_Info object
    /// \return true if the two Type_Info objects are equal
    /// \return false if the two Type_Info objects are not equal
    constexpr bool bare_equal(const Type_Info &ti) const noexcept {
        return ti.m_bare_type_info == m_bare_type_info ||
               *ti.m_bare_type_info == *m_bare_type_info;
    }

    /// \brief Compare two Type_Info objects
    /// \param ti The other Type_Info object
    /// \return true if the two Type_Info objects are equal
    /// \return false if the two Type_Info objects are not equal
    constexpr bool bare_equal_type_info(
        const std::type_info &ti) const noexcept {
        return !is_undef() && (*m_bare_type_info) == ti;
    }

    /// \brief Check if the type is const
    constexpr bool is_const() const noexcept {
        return (m_flags & (1 << is_const_flag)) != 0;
    }
    /// Check if the type is a reference
    constexpr bool is_reference() const noexcept {
        return (m_flags & (1 << is_reference_flag)) != 0;
    }

    /// Check if the type is void
    constexpr bool is_void() const noexcept {
        return (m_flags & (1 << is_void_flag)) != 0;
    }

    /// Check if the type is arithmetic
    constexpr bool is_arithmetic() const noexcept {
        return (m_flags & (1 << is_arithmetic_flag)) != 0;
    }

    /// Check if the type is undef
    constexpr bool is_undef() const noexcept {
        return (m_flags & (1 << is_undef_flag)) != 0;
    }

    /// Check if the type is a pointer
    constexpr bool is_pointer() const noexcept {
        return (m_flags & (1 << is_pointer_flag)) != 0;
    }

    /// Get the name of the type
    const char *name() const noexcept {
        if (!is_undef()) {
            return m_type_info->name();
        } else {
            return "";
        }
    }

    /// Get the name of the bare type
    const char *bare_name() const noexcept {
        if (!is_undef()) {
            return m_bare_type_info->name();
        } else {
            return "";
        }
    }

    /// Get the type_info of the bare type
    constexpr const std::type_info *bare_type_info() const noexcept {
        return m_bare_type_info;
    }

private:
    struct Unknown_Type {};

    const std::type_info *m_type_info = &typeid(Unknown_Type);
    const std::type_info *m_bare_type_info = &typeid(Unknown_Type);
    static const int is_const_flag = 0;
    static const int is_reference_flag = 1;
    static const int is_pointer_flag = 2;
    static const int is_void_flag = 3;
    static const int is_arithmetic_flag = 4;
    static const int is_undef_flag = 5;
    unsigned int m_flags = (1 << is_undef_flag);
};

/// Helper used to create a Type_Info object
template <typename T>
struct Get_Type_Info {
    constexpr static Type_Info get() noexcept {
        return Type_Info(
            std::is_const<typename std::remove_pointer<
                typename std::remove_reference<T>::type>::type>::value,
            std::is_reference<T>::value, std::is_pointer<T>::value,
            std::is_void<T>::value,
            (std::is_arithmetic<T>::value ||
             std::is_arithmetic<
                 typename std::remove_reference<T>::type>::value) &&
                !std::is_same<
                    typename std::remove_const<
                        typename std::remove_reference<T>::type>::type,
                    bool>::value,
            &typeid(T), &typeid(typename Bare_Type<T>::type));
    }
};

template <typename T>
struct Get_Type_Info<std::shared_ptr<T>> {
    constexpr static Type_Info get() noexcept {
        return Type_Info(
            std::is_const<T>::value, std::is_reference<T>::value,
            std::is_pointer<T>::value, std::is_void<T>::value,
            std::is_arithmetic<T>::value &&
                !std::is_same<
                    typename std::remove_const<
                        typename std::remove_reference<T>::type>::type,
                    bool>::value,
            &typeid(std::shared_ptr<T>), &typeid(typename Bare_Type<T>::type));
    }
};

template <typename T>
struct Get_Type_Info<std::shared_ptr<T> &> : Get_Type_Info<std::shared_ptr<T>> {
};

template <typename T>
struct Get_Type_Info<const std::shared_ptr<T> &> {
    constexpr static Type_Info get() noexcept {
        return Type_Info(
            std::is_const<T>::value, std::is_reference<T>::value,
            std::is_pointer<T>::value, std::is_void<T>::value,
            std::is_arithmetic<T>::value &&
                !std::is_same<
                    typename std::remove_const<
                        typename std::remove_reference<T>::type>::type,
                    bool>::value,
            &typeid(const std::shared_ptr<T> &),
            &typeid(typename Bare_Type<T>::type));
    }
};

template <typename T>
struct Get_Type_Info<std::reference_wrapper<T>> {
    constexpr static Type_Info get() noexcept {
        return Type_Info(
            std::is_const<T>::value, std::is_reference<T>::value,
            std::is_pointer<T>::value, std::is_void<T>::value,
            std::is_arithmetic<T>::value &&
                !std::is_same<
                    typename std::remove_const<
                        typename std::remove_reference<T>::type>::type,
                    bool>::value,
            &typeid(std::reference_wrapper<T>),
            &typeid(typename Bare_Type<T>::type));
    }
};

template <typename T>
struct Get_Type_Info<const std::reference_wrapper<T> &> {
    constexpr static Type_Info get() noexcept {
        return Type_Info(
            std::is_const<T>::value, std::is_reference<T>::value,
            std::is_pointer<T>::value, std::is_void<T>::value,
            std::is_arithmetic<T>::value &&
                !std::is_same<
                    typename std::remove_const<
                        typename std::remove_reference<T>::type>::type,
                    bool>::value,
            &typeid(const std::reference_wrapper<T> &),
            &typeid(typename Bare_Type<T>::type));
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

#endif
