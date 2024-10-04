#ifndef ATOM_META_PROPERTY_HPP
#define ATOM_META_PROPERTY_HPP

#include <functional>
#include <optional>

#include "atom/error/exception.hpp"
#include "concept.hpp"

/**
 * @brief A template class that encapsulates a property with optional getter,
 * setter, and onChange callback.
 *
 * @tparam T The type of the property value. Must satisfy the Copyable concept.
 */
template <Copyable T>
class Property {
private:
    std::optional<T>
        value_;  ///< The value of the property, using std::optional to handle
                 ///< default-initialized state.
    std::function<T()> getter_;      ///< The getter function for the property.
    std::function<void(T)> setter_;  ///< The setter function for the property.
    std::function<void(const T&)>
        onChange_;  ///< The onChange callback function for the property.

public:
    /**
     * @brief Default constructor.
     */
    Property() = default;

    /**
     * @brief Constructor that initializes the property with a getter function.
     *
     * @param get The getter function.
     */
    explicit Property(std::function<T()> get) : getter_(std::move(get)) {}

    /**
     * @brief Constructor that initializes the property with a getter and setter
     * function.
     *
     * @param get The getter function.
     * @param set The setter function.
     */
    Property(std::function<T()> get, std::function<void(T)> set)
        : getter_(std::move(get)), setter_(std::move(set)) {}

    /**
     * @brief Constructor that initializes the property with a default value.
     *
     * @param defaultValue The default value of the property.
     */
    explicit Property(const T& defaultValue) : value_(defaultValue) {}

    /**
     * @brief Destructor.
     */
    ~Property() = default;

    /**
     * @brief Copy constructor.
     *
     * @param other The other Property object to copy from.
     */
    Property(const Property& other)
        : value_(other.value_),
          getter_(other.getter_),
          setter_(other.setter_),
          onChange_(other.onChange_) {}

    /**
     * @brief Copy assignment operator.
     *
     * @param other The other Property object to copy from.
     * @return Property& A reference to this Property object.
     */
    auto operator=(const Property& other) -> Property& {
        if (this != &other) {
            value_ = other.value_;
            getter_ = other.getter_;
            setter_ = other.setter_;
            onChange_ = other.onChange_;
        }
        return *this;
    }

    /**
     * @brief Move constructor.
     *
     * @param other The other Property object to move from.
     */
    Property(Property&& other) noexcept
        : value_(std::move(other.value_)),
          getter_(std::move(other.getter_)),
          setter_(std::move(other.setter_)),
          onChange_(std::move(other.onChange_)) {}

    /**
     * @brief Move assignment operator.
     *
     * @param other The other Property object to move from.
     * @return Property& A reference to this Property object.
     */
    auto operator=(Property&& other) noexcept -> Property& {
        if (this != &other) {
            value_ = std::move(other.value_);
            getter_ = std::move(other.getter_);
            setter_ = std::move(other.setter_);
            onChange_ = std::move(other.onChange_);
        }
        return *this;
    }

    /**
     * @brief Conversion operator to the underlying type T.
     *
     * @return T The value of the property.
     * @throws std::invalid_argument if neither value nor getter is defined.
     */
    explicit operator T() const {
        if (getter_) {
            return getter_();
        }
        if (value_) {
            return *value_;
        }
        THROW_INVALID_ARGUMENT("Property has no value or getter defined");
    }

    /**
     * @brief Assignment operator for the underlying type T.
     *
     * @param newValue The new value to set.
     * @return Property& A reference to this Property object.
     */
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

    /**
     * @brief Sets the property to readonly by removing the setter function.
     */
    void makeReadonly() { setter_ = nullptr; }

    /**
     * @brief Sets the property to writeonly by removing the getter function.
     */
    void makeWriteonly() { getter_ = nullptr; }

    /**
     * @brief Removes both getter and setter functions.
     */
    void clear() {
        getter_ = nullptr;
        setter_ = nullptr;
    }

    /**
     * @brief Sets the onChange callback function.
     *
     * @param callback The onChange callback function.
     */
    void setOnChange(std::function<void(const T&)> callback) {
        onChange_ = std::move(callback);
    }

    /**
     * @brief Stream output operator for the Property class.
     *
     * @param outputStream The output stream.
     * @param prop The Property object to output.
     * @return std::ostream& The output stream.
     */
    friend auto operator<<(std::ostream& outputStream,
                           const Property& prop) -> std::ostream& {
        outputStream << static_cast<T>(prop);
        return outputStream;
    }

    /**
     * @brief Three-way comparison operator.
     *
     * @param other The other value to compare with.
     * @return std::strong_ordering The result of the comparison.
     */
    auto operator<=>(const T& other) const {
        return static_cast<T>(*this) <=> other;
    }

    /**
     * @brief Equality comparison operator.
     *
     * @param other The other value to compare with.
     * @return bool True if equal, false otherwise.
     */
    auto operator==(const T& other) const -> bool {
        return static_cast<T>(*this) == other;
    }

    /**
     * @brief Inequality comparison operator.
     *
     * @param other The other value to compare with.
     * @return bool True if not equal, false otherwise.
     */
    auto operator!=(const T& other) const -> bool { return !(*this == other); }

    /**
     * @brief Addition assignment operator.
     *
     * @param other The other value to add.
     * @return Property& A reference to this Property object.
     */
    auto operator+=(const T& other) -> Property& {
        *this = static_cast<T>(*this) + other;
        return *this;
    }

    /**
     * @brief Subtraction assignment operator.
     *
     * @param other The other value to subtract.
     * @return Property& A reference to this Property object.
     */
    auto operator-=(const T& other) -> Property& {
        *this = static_cast<T>(*this) - other;
        return *this;
    }

    /**
     * @brief Multiplication assignment operator.
     *
     * @param other The other value to multiply.
     * @return Property& A reference to this Property object.
     */
    auto operator*=(const T& other) -> Property& {
        *this = static_cast<T>(*this) * other;
        return *this;
    }

    /**
     * @brief Division assignment operator.
     *
     * @param other The other value to divide.
     * @return Property& A reference to this Property object.
     */
    auto operator/=(const T& other) -> Property& {
        *this = static_cast<T>(*this) / other;
        return *this;
    }

    /**
     * @brief Modulus assignment operator.
     *
     * @param other The other value to modulus.
     * @return Property& A reference to this Property object.
     */
    auto operator%=(const T& other) -> Property& {
        *this = static_cast<T>(*this) % other;
        return *this;
    }
};

/**
 * @brief Macro to define a read-write property.
 *
 * @param Type The type of the property.
 * @param Name The name of the property.
 */
#define DEFINE_RW_PROPERTY(Type, Name)               \
private:                                             \
    Type Name##_;                                    \
                                                     \
public:                                              \
    Property<Type>(Name) =                           \
        Property<Type>([this]() { return Name##_; }, \
                       [this](const Type& value) { Name##_ = value; });

/**
 * @brief Macro to define a read-only property.
 *
 * @param Type The type of the property.
 * @param Name The name of the property.
 */
#define DEFINE_RO_PROPERTY(Type, Name) \
private:                               \
    Type Name##_;                      \
                                       \
public:                                \
    Property<Type>(Name) = Property<Type>([this]() { return Name##_; });

/**
 * @brief Macro to define a write-only property.
 *
 * @param Type The type of the property.
 * @param Name The name of the property.
 */
#define DEFINE_WO_PROPERTY(Type, Name)     \
private:                                   \
    Type Name##_;                          \
                                           \
public:                                    \
    Property<Type>(Name) = Property<Type>( \
        nullptr, [this](const Type& value) { Name##_ = value; });

#endif