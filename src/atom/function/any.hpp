#ifndef ATOM_FUNCTION_ANY_HPP
#define ATOM_FUNCTION_ANY_HPP

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

#include "type_info.hpp"

class BoxedValue {
public:
    struct Void_Type {};

private:
    struct Data {
        std::any m_obj;
        Type_Info m_type_info;
        std::shared_ptr<std::map<std::string, std::shared_ptr<Data>>> m_attrs;
        bool m_is_ref = false;
        bool m_return_value = false;
        bool m_readonly = false;
        const void* m_const_data_ptr = nullptr;

        template <typename T>
        Data(T&& obj, bool is_ref, bool return_value, bool readonly)
            : m_obj(std::forward<T>(obj)),
              m_type_info(user_type<T>()),
              m_attrs(nullptr),
              m_is_ref(is_ref),
              m_return_value(return_value),
              m_readonly(readonly),
              m_const_data_ptr((const void*)&obj) {}
    };

    std::shared_ptr<Data> m_data;
    mutable std::mutex m_mutex;

public:
    template <typename T, typename = std::enable_if_t<
                              !std::is_same_v<BoxedValue, std::decay_t<T>>>>
    explicit BoxedValue(T&& t, bool t_return_value = false,
                        bool readonly = false)
        : m_data(std::make_shared<Data>(std::forward<T>(t),
                                        std::is_reference_v<T>, t_return_value,
                                        readonly)) {}

    BoxedValue()
        : m_data(std::make_shared<Data>(Void_Type{}, false, false, false)) {}

    BoxedValue(const BoxedValue& other) : m_data(other.m_data) {}
    BoxedValue(BoxedValue&& other) noexcept : m_data(std::move(other.m_data)) {}

    BoxedValue& operator=(const BoxedValue& other) {
        if (this != &other) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data = other.m_data;
        }
        return *this;
    }

    BoxedValue& operator=(BoxedValue&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data = std::move(other.m_data);
        }
        return *this;
    }

    void swap(BoxedValue& rhs) noexcept { std::swap(m_data, rhs.m_data); }

    bool is_undef() const noexcept {
        return m_data->m_obj.type() == typeid(Void_Type);
    }

    bool is_const() const noexcept { return m_data->m_type_info.is_const(); }

    bool is_type(const Type_Info& ti) const noexcept {
        return m_data->m_type_info == ti;
    }

    bool is_ref() const noexcept { return m_data->m_is_ref; }

    bool is_return_value() const noexcept { return m_data->m_return_value; }

    void reset_return_value() noexcept { m_data->m_return_value = false; }

    bool is_readonly() const noexcept { return m_data->m_readonly; }

    bool is_const_data_ptr() const noexcept {
        return m_data->m_const_data_ptr != nullptr;
    }

    const std::any& get() const noexcept { return m_data->m_obj; }

    const Type_Info& get_type_info() const noexcept {
        return m_data->m_type_info;
    }

    BoxedValue& set_attr(const std::string& name, const BoxedValue& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_data->m_attrs) {
            m_data->m_attrs = std::make_shared<
                std::map<std::string, std::shared_ptr<Data>>>();
        }
        (*m_data->m_attrs)[name] = value.m_data;
        return *this;
    }

    BoxedValue get_attr(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_data->m_attrs) {
            auto it = m_data->m_attrs->find(name);
            if (it != m_data->m_attrs->end()) {
                return BoxedValue(it->second);
            }
        }
        return BoxedValue();  // Return undefined BoxedValue
    }

    bool has_attr(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_data->m_attrs &&
               (m_data->m_attrs->find(name) != m_data->m_attrs->end());
    }

    void remove_attr(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_data->m_attrs) {
            m_data->m_attrs->erase(name);
        }
    }

    std::vector<std::string> list_attrs() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> attrs;
        if (m_data->m_attrs) {
            for (const auto& entry : *m_data->m_attrs) {
                attrs.push_back(entry.first);
            }
        }
        return attrs;
    }

    bool is_null() const noexcept {
        if (m_data->m_obj.has_value()) {
            try {
                return std::any_cast<std::nullptr_t>(m_data->m_obj) != nullptr;
            } catch (const std::bad_any_cast&) {
                return false;
            }
        }
        return true;
    }

    void* get_ptr() const noexcept {
        return const_cast<void*>(m_data->m_const_data_ptr);
    }

    /// Attempt to cast the stored value to the given type, T, and return it
    template <typename T>
    std::optional<T> try_cast() const noexcept {
        try {
            if constexpr (std::is_reference_v<T>) {
                if (m_data->m_obj.type() == typeid(std::reference_wrapper<T>)) {
                    auto& ref =
                        std::any_cast<std::reference_wrapper<T>>(m_data->m_obj);
                    return ref.get();
                }
            }
            return std::any_cast<T>(m_data->m_obj);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    /// Check if the stored value can be cast to the given type, T
    template <typename T>
    bool can_cast() const noexcept {
        try {
            if constexpr (std::is_reference_v<T>) {
                if (m_data->m_obj.type() == typeid(std::reference_wrapper<T>)) {
                    return true;
                }
            } else {
                std::any_cast<T>(m_data->m_obj);
                return true;
            }
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }

    /// Debug string representation of the contained object
    std::string debug_string() const {
        std::ostringstream oss;
        oss << "BoxedValue<" << m_data->m_type_info.name() << ">: ";
        try {
            if (m_data->m_obj.type() == typeid(int))
                oss << std::any_cast<int>(m_data->m_obj);
            else if (m_data->m_obj.type() == typeid(double))
                oss << std::any_cast<double>(m_data->m_obj);
            else if (m_data->m_obj.type() == typeid(std::string))
                oss << std::any_cast<std::string>(m_data->m_obj);
            else if (m_data->m_obj.type() == typeid(bool))
                oss << std::any_cast<bool>(m_data->m_obj);
            else if (m_data->m_obj.type() == typeid(std::vector<int>))
                oss << "vector<int>";
            else if (m_data->m_obj.type() == typeid(std::vector<double>))
                oss << "vector<double>";
            else if (m_data->m_obj.type() == typeid(std::vector<std::string>))
                oss << "vector<string>";
            else if (m_data->m_obj.type() == typeid(std::vector<bool>))
                oss << "vector<bool>";
            else
                oss << "unknown type";
        } catch (const std::bad_any_cast&) {
            oss << "failed to cast";
        }
        return oss.str();
    }

    /// Visitor pattern implementation
    template <typename Visitor>
    auto visit(Visitor&& visitor) -> decltype(auto) {
        return std::visit(std::forward<Visitor>(visitor), m_data->m_obj);
    }
};

// Helper function to create a non-constant BoxedValue
template <typename T>
BoxedValue var(T&& t) {
    return BoxedValue(std::forward<T>(t));
}

// Helper function to create a constant BoxedValue
template <typename T>
BoxedValue const_var(const T& t) {
    return BoxedValue(std::cref(t), false, true);
}

// Helper function to create一个void BoxedValue
inline BoxedValue void_var() { return BoxedValue(BoxedValue::Void_Type{}); }

#endif  // ATOM_FUNCTION_ANY_HPP