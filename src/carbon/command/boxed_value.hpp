#ifndef CARBON_BOXED_VALUE_HPP
#define CARBON_BOXED_VALUE_HPP

#include <map>
#include <memory>
#include <type_traits>

#include "../defines.hpp"
#include "any.hpp"
#include "atom/function/type_info.hpp"
#include "atom/type/pointer.hpp"

namespace Carbon {
/// \brief A wrapper for holding any valid C++ type. All types in ChaiScript are
/// Boxed_Value objects \sa Carbon::boxed_cast
class Boxed_Value {
public:
    /// used for explicitly creating a "void" object
    struct Void_Type {};

private:
    /// structure which holds the internal state of a Boxed_Value
    /// \todo Get rid of Any and merge it with this, reducing an allocation in
    /// the process
    struct Data {
        Data(const atom::meta::Type_Info &ti, Carbon::detail::Any to, bool is_ref,
             const void *t_void_ptr, bool t_return_value);

        Data &operator=(const Data &rhs);

        Data(const Data &) = delete;

        Data(Data &&) = default;
        Data &operator=(Data &&rhs) = default;

        atom::meta::Type_Info m_type_info;
        Carbon::detail::Any m_obj;
        void *m_data_ptr;
        const void *m_const_data_ptr;
        std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>> m_attrs;
        bool m_is_ref;
        bool m_return_value;
    };

    struct Object_Data {
        static auto get(Boxed_Value::Void_Type, bool t_return_value) {
            return std::make_shared<Data>(atom::meta::Get_Type_Info<void>::get(),
                                          Carbon::detail::Any(), false,
                                          nullptr, t_return_value);
        }

        template <typename T>
        static auto get(const std::shared_ptr<T> *obj, bool t_return_value) {
            return get(*obj, t_return_value);
        }

        template <typename T>
        static auto get(const std::shared_ptr<T> &obj, bool t_return_value) {
            return std::make_shared<Data>(atom::meta::Get_Type_Info<T>::get(),
                                          Carbon::detail::Any(obj), false,
                                          obj.get(), t_return_value);
        }

        template <typename T>
        static auto get(std::shared_ptr<T> &&obj, bool t_return_value) {
            auto ptr = obj.get();
            return std::make_shared<Data>(
                atom::meta::Get_Type_Info<T>::get(),
                Carbon::detail::Any(std::move(obj)), false, ptr,
                t_return_value);
        }

        template <typename T>
        static auto get(T *t, bool t_return_value) {
            return get(std::ref(*t), t_return_value);
        }

        template <typename T>
        static auto get(const T *t, bool t_return_value) {
            return get(std::cref(*t), t_return_value);
        }

        template <typename T>
        static auto get(std::reference_wrapper<T> obj, bool t_return_value) {
            auto p = &obj.get();
            return std::make_shared<Data>(
                atom::meta::Get_Type_Info<T>::get(),
                Carbon::detail::Any(std::move(obj)), true, p,
                t_return_value);
        }

        template <typename T>
        static auto get(std::unique_ptr<T> &&obj, bool t_return_value) {
            auto ptr = obj.get();
            return std::make_shared<Data>(
                atom::meta::Get_Type_Info<T>::get(),
                Carbon::detail::Any(
                    std::make_shared<std::unique_ptr<T>>(std::move(obj))),
                true, ptr, t_return_value);
        }

        template <typename T>
        static auto get(T t, bool t_return_value) {
            auto p = std::make_shared<T>(std::move(t));
            auto ptr = p.get();
            return std::make_shared<Data>(atom::meta::Get_Type_Info<T>::get(),
                                          Carbon::detail::Any(std::move(p)),
                                          false, ptr, t_return_value);
        }

        static std::shared_ptr<Data> get() {
            return std::make_shared<Data>(
                atom::meta::Type_Info(), Carbon::detail::Any(), false, nullptr, false);
        }
    };

public:
    /// Basic Boxed_Value constructor
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<Boxed_Value, std::decay_t<T>>>>
    explicit Boxed_Value(T &&t, bool t_return_value = false)
        : m_data(Object_Data::get(std::forward<T>(t), t_return_value)) {}

    /// Unknown-type constructor
    Boxed_Value() = default;

    Boxed_Value(Boxed_Value &&) = default;
    Boxed_Value &operator=(Boxed_Value &&) = default;
    Boxed_Value(const Boxed_Value &) = default;
    Boxed_Value &operator=(const Boxed_Value &) = default;

    void swap(Boxed_Value &rhs);

    /// Copy the values stored in rhs.m_data to m_data.
    /// m_data pointers are not shared in this case
    Boxed_Value assign(const Boxed_Value &rhs) noexcept;

    const atom::meta::Type_Info &get_type_info() const noexcept;

    /// return true if the object is uninitialized
    bool is_undef() const noexcept;

    bool is_const() const noexcept;

    bool is_type(const atom::meta::Type_Info &ti) const noexcept;

    template <typename T>
    auto pointer_sentinel(std::shared_ptr<T> &ptr) const noexcept {
        struct Sentinel {
            Sentinel(std::shared_ptr<T> &t_ptr, Data &data)
                : m_ptr(t_ptr), m_data(data) {}

            ~Sentinel() {
                // save new pointer data
                const auto ptr_ = m_ptr.get().get();
                m_data.get().m_data_ptr = ptr_;
                m_data.get().m_const_data_ptr = ptr_;
            }

            Sentinel &operator=(Sentinel &&s) = default;
            Sentinel(Sentinel &&s) = default;

            operator std::shared_ptr<T> &() const noexcept {
                return m_ptr.get();
            }

            Sentinel &operator=(const Sentinel &) = delete;
            Sentinel(Sentinel &) = delete;

            std::reference_wrapper<std::shared_ptr<T>> m_ptr;
            std::reference_wrapper<Data> m_data;
        };

        return Sentinel(ptr, *(m_data.get()));
    }

    bool is_null() const noexcept;

    const Carbon::detail::Any &get() const noexcept;

    bool is_ref() const noexcept;

    bool is_return_value() const noexcept;

    void reset_return_value() const noexcept;

    bool is_pointer() const noexcept;

    void *get_ptr() const noexcept;

    const void *get_const_ptr() const noexcept;

    Boxed_Value get_attr(const std::string &t_name);

    Boxed_Value &copy_attrs(const Boxed_Value &t_obj);

    Boxed_Value &clone_attrs(const Boxed_Value &t_obj);

    /// \returns true if the two Boxed_Values share the same internal type
    static bool type_match(const Boxed_Value &l,
                           const Boxed_Value &r) noexcept;

private:
    // necessary to avoid hitting the templated && constructor of Boxed_Value
    struct Internal_Construction {};

    Boxed_Value(std::shared_ptr<Data> t_data, Internal_Construction);

    std::shared_ptr<Data> m_data = Object_Data::get();
};

/// @brief Creates a Boxed_Value. If the object passed in is a value type, it is
/// copied. If it is a pointer, std::shared_ptr, or std::reference_type
///        a copy is not made.
/// @param t The value to box
///
/// Example:
///
/// ~~~{.cpp}
/// int i;
/// Carbon::ChaiScript chai;
/// chai.add(Carbon::var(i), "i");
/// chai.add(Carbon::var(&i), "ip");
/// ~~~
///
/// @sa @ref adding_objects
template <typename T>
Boxed_Value var(T &&t) {
    return Boxed_Value(std::forward<T>(t));
}

namespace detail {
/// \brief Takes a value, copies it and returns a Boxed_Value object that is
/// immutable \param[in] t Value to copy and make const \returns Immutable
/// Boxed_Value \sa Boxed_Value::is_const
template <typename T>
Boxed_Value const_var_impl(const T &t) {
    return Boxed_Value(std::make_shared<typename std::add_const<T>::type>(t));
}

/// \brief Takes a pointer to a value, adds const to the pointed to type and
/// returns an immutable Boxed_Value.
///        Does not copy the pointed to value.
/// \param[in] t Pointer to make immutable
/// \returns Immutable Boxed_Value
/// \sa Boxed_Value::is_const
template <typename T>
Boxed_Value const_var_impl(T *t) {
    return Boxed_Value(const_cast<typename std::add_const<T>::type *>(t));
}

/// \brief Takes a std::shared_ptr to a value, adds const to the pointed to type
/// and returns an immutable Boxed_Value.
///        Does not copy the pointed to value.
/// \param[in] t Pointer to make immutable
/// \returns Immutable Boxed_Value
/// \sa Boxed_Value::is_const
template <typename T>
Boxed_Value const_var_impl(const std::shared_ptr<T> &t) {
    return Boxed_Value(
        std::const_pointer_cast<typename std::add_const<T>::type>(t));
}

/// \brief Takes a std::reference_wrapper value, adds const to the referenced
/// type and returns an immutable Boxed_Value.
///        Does not copy the referenced value.
/// \param[in] t Reference object to make immutable
/// \returns Immutable Boxed_Value
/// \sa Boxed_Value::is_const
template <typename T>
Boxed_Value const_var_impl(const std::reference_wrapper<T> &t) {
    return Boxed_Value(std::cref(t.get()));
}
}  // namespace detail

/// \brief Takes an object and returns an immutable Boxed_Value. If the object
/// is a std::reference or pointer type
///        the value is not copied. If it is an object type, it is copied.
/// \param[in] t Object to make immutable
/// \returns Immutable Boxed_Value
/// \sa Carbon::Boxed_Value::is_const
/// \sa Carbon::var
///
/// Example:
/// \code
/// enum Colors
/// {
///   Blue,
///   Green,
///   Red
/// };
/// Carbon::ChaiScript chai
/// chai.add(Carbon::const_var(Blue), "Blue"); // add immutable constant
/// chai.add(Carbon::const_var(Red), "Red");
/// chai.add(Carbon::const_var(Green), "Green");
/// \endcode
///
/// \todo support C++11 strongly typed enums
/// \sa \ref adding_objects
template <typename T>
Boxed_Value const_var(const T &t) {
    return detail::const_var_impl(t);
}

inline Boxed_Value void_var() {
    static const auto v = Boxed_Value(Boxed_Value::Void_Type());
    return v;
}

inline Boxed_Value const_var(bool b) {
    static const auto t = detail::const_var_impl(true);
    static const auto f = detail::const_var_impl(false);

    if (b) {
        return t;
    } else {
        return f;
    }
}

}  // namespace Carbon

#endif
