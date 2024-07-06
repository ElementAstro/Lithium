#ifndef ATOM_META_PROPERTY_HPP
#define ATOM_META_PROPERTY_HPP

#include <functional>
#include <optional>

#include "atom/error/exception.hpp"
#include "concept.hpp"

template <Copyable T>
class Property {
private:
    std::optional<T>
        value_;  // Use std::optional to handle default-initialized state
    std::function<T()> getter_;
    std::function<void(T)> setter_;
    std::function<void(const T&)> onChange_;

public:
    Property() = default;

    Property(std::function<T()> get) : getter_(std::move(get)) {}

    Property(std::function<T()> get, std::function<void(T)> set)
        : getter_(std::move(get)), setter_(std::move(set)) {}

    Property(const T& defaultValue) : value_(defaultValue) {}

    // Move constructor
    Property(Property&& other) noexcept
        : value_(std::move(other.value_)),
          getter_(std::move(other.getter_)),
          setter_(std::move(other.setter_)),
          onChange_(std::move(other.onChange_)) {}

    // Move assignment operator
    auto operator=(Property&& other) noexcept -> Property& {
        if (this != &other) {
            value_ = std::move(other.value_);
            getter_ = std::move(other.getter_);
            setter_ = std::move(other.setter_);
            onChange_ = std::move(other.onChange_);
        }
        return *this;
    }

    operator T() const {
        if (getter_) {
            return getter_();
        }
        if (value_) {
            return *value_;
        }
        THROW_INVALID_ARGUMENT("Property has no value or getter defined");
    }

    auto operator=(const T& newValue) -> Property& {
        if (setter_) {
            setter_(newValue);
        } else {
            value_ = newValue;
        }
        if (onChange_) {
            onChange_(newValue);
        }
        return *this;
    }

    // Set property to readonly
    void makeReadonly() { setter_ = nullptr; }

    // Set property to writeonly
    void makeWriteonly() { getter_ = nullptr; }

    // Remove getter and setter
    void clear() {
        getter_ = nullptr;
        setter_ = nullptr;
    }

    // Set onChange callback
    void setOnChange(std::function<void(const T&)> callback) {
        onChange_ = std::move(callback);
    }

    // Friend function for stream output
    friend auto operator<<(std::ostream& os,
                           const Property& prop) -> std::ostream& {
        os << static_cast<T>(prop);
        return os;
    }

    // Comparison operators
    auto operator<=>(const T& other) const {
        return static_cast<T>(*this) <=> other;
    }

    // 对于 == 和 != 运算符，我们仍然需要显式定义
    auto operator==(const T& other) const -> bool {
        return static_cast<T>(*this) == other;
    }
    auto operator!=(const T& other) const -> bool { return !(*this == other); }

    // Arithmetic operators
    auto operator+=(const T& other) -> Property& {
        *this = static_cast<T>(*this) + other;
        return *this;
    }

    auto operator-=(const T& other) -> Property& {
        *this = static_cast<T>(*this) - other;
        return *this;
    }

    auto operator*=(const T& other) -> Property& {
        *this = static_cast<T>(*this) * other;
        return *this;
    }

    auto operator/=(const T& other) -> Property& {
        *this = static_cast<T>(*this) / other;
        return *this;
    }

    auto operator%=(const T& other) -> Property& {
        *this = static_cast<T>(*this) % other;
        return *this;
    }
};

// Macro to define a read-write property
#define DEFINE_RW_PROPERTY(Type, Name)               \
private:                                             \
    Type Name##_;                                    \
                                                     \
public:                                              \
    Property<Type>(Name) =                           \
        Property<Type>([this]() { return Name##_; }, \
                       [this](const Type& value) { Name##_ = value; });

// Macro to define a read-only property
#define DEFINE_RO_PROPERTY(Type, Name) \
private:                               \
    Type Name##_;                      \
                                       \
public:                                \
    Property<Type>(Name) = Property<Type>([this]() { return Name##_; });

// Macro to define a write-only property
#define DEFINE_WO_PROPERTY(Type, Name)     \
private:                                   \
    Type Name##_;                          \
                                           \
public:                                    \
    Property<Type>(Name) = Property<Type>( \
        nullptr, [this](const Type& value) { Name##_ = value; });

#endif