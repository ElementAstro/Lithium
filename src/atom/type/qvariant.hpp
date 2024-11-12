#ifndef ATOM_TYPE_QVARIANT_HPP
#define ATOM_TYPE_QVARIANT_HPP

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <variant>

namespace atom::type {
/**
 * @brief A wrapper class for std::variant with additional utility functions.
 *
 * @tparam Types The types that the variant can hold.
 */
template <typename... Types>
class VariantWrapper {
public:
    using VariantType = std::variant<std::monostate, Types...>;

    /**
     * @brief Default constructor.
     */
    VariantWrapper();

    /**
     * @brief Constructs a VariantWrapper with an initial value.
     *
     * @tparam T The type of the initial value.
     * @param value The initial value to store in the variant.
     */
    template <typename T>
    explicit VariantWrapper(T&& value);

    /**
     * @brief Copy constructor.
     *
     * @param other The other VariantWrapper to copy from.
     */
    VariantWrapper(const VariantWrapper& other);

    /**
     * @brief Move constructor.
     *
     * @param other The other VariantWrapper to move from.
     */
    VariantWrapper(VariantWrapper&& other) noexcept;

    /**
     * @brief Copy assignment operator.
     *
     * @param other The other VariantWrapper to copy from.
     * @return A reference to this VariantWrapper.
     */
    auto operator=(const VariantWrapper& other) -> VariantWrapper&;

    /**
     * @brief Move assignment operator.
     *
     * @param other The other VariantWrapper to move from.
     * @return A reference to this VariantWrapper.
     */
    auto operator=(VariantWrapper&& other) noexcept -> VariantWrapper&;

    /**
     * @brief Assignment operator for a value.
     *
     * @tparam T The type of the value.
     * @param value The value to assign to the variant.
     * @return A reference to this VariantWrapper.
     */
    template <typename T>
    auto operator=(T&& value) -> VariantWrapper&;

    /**
     * @brief Gets the name of the type currently held by the variant.
     *
     * @return The name of the type as a string.
     */
    [[nodiscard]] auto typeName() const -> std::string;

    /**
     * @brief Gets the value of the specified type from the variant.
     *
     * @tparam T The type of the value to get.
     * @return The value of the specified type.
     * @throws std::bad_variant_access if the variant does not hold the
     * specified type.
     */
    template <typename T>
    auto get() const -> T;

    /**
     * @brief Checks if the variant holds the specified type.
     *
     * @tparam T The type to check.
     * @return True if the variant holds the specified type, false otherwise.
     */
    template <typename T>
    [[nodiscard]] auto is() const -> bool;

    /**
     * @brief Prints the current value of the variant to the standard output.
     */
    void print() const;

    /**
     * @brief Equality operator.
     *
     * @param other The other VariantWrapper to compare with.
     * @return True if the variants are equal, false otherwise.
     */
    [[nodiscard]] auto operator==(const VariantWrapper& other) const -> bool;

    /**
     * @brief Inequality operator.
     *
     * @param other The other VariantWrapper to compare with.
     * @return True if the variants are not equal, false otherwise.
     */
    [[nodiscard]] auto operator!=(const VariantWrapper& other) const -> bool;

    /**
     * @brief Visits the variant with a visitor.
     *
     * @tparam Visitor The type of the visitor.
     * @param visitor The visitor to apply to the variant.
     * @return The result of the visitor.
     */
    template <typename Visitor>
    auto visit(Visitor&& visitor) const -> decltype(auto);

    /**
     * @brief Gets the index of the currently held type in the variant.
     *
     * @return The index of the currently held type.
     */
    [[nodiscard]] auto index() const -> std::size_t;

    /**
     * @brief Tries to get the value of the specified type from the variant.
     *
     * @tparam T The type of the value to get.
     * @return An optional containing the value if the variant holds the
     * specified type, std::nullopt otherwise.
     */
    template <typename T>
    auto tryGet() const -> std::optional<T>;

    /**
     * @brief Tries to convert the current value to an int.
     *
     * @return An optional containing the int value if the conversion is
     * successful, std::nullopt otherwise.
     */
    [[nodiscard]] auto toInt() const -> std::optional<int>;

    /**
     * @brief Tries to convert the current value to a double.
     *
     * @return An optional containing the double value if the conversion is
     * successful, std::nullopt otherwise.
     */
    [[nodiscard]] auto toDouble() const -> std::optional<double>;

    /**
     * @brief Tries to convert the current value to a bool.
     *
     * @return An optional containing the bool value if the conversion is
     * successful, std::nullopt otherwise.
     */
    [[nodiscard]] auto toBool() const -> std::optional<bool>;

    /**
     * @brief Converts the current value to a string.
     *
     * @return The string representation of the current value.
     */
    [[nodiscard]] auto toString() const -> std::string;

    /**
     * @brief Resets the variant to hold std::monostate.
     */
    void reset();

    /**
     * @brief Checks if the variant holds a value other than std::monostate.
     *
     * @return True if the variant holds a value, false otherwise.
     */
    [[nodiscard]] auto hasValue() const -> bool;

    /**
     * @brief Stream insertion operator for VariantWrapper.
     *
     * @param outputStream The output stream.
     * @param variantWrapper The VariantWrapper to insert into the stream.
     * @return The output stream.
     */
    friend auto operator<<(std::ostream& outputStream,
                           const VariantWrapper& variantWrapper)
        -> std::ostream&;

    /**
     * @brief Default destructor.
     */
    ~VariantWrapper() = default;

private:
    VariantType variant_ = VariantType(std::in_place_index<0>);
};

// 实现部分

template <typename... Types>
VariantWrapper<Types...>::VariantWrapper() = default;

template <typename... Types>
template <typename T>
VariantWrapper<Types...>::VariantWrapper(T&& value)
    : variant_(std::forward<T>(value)) {}

template <typename... Types>
VariantWrapper<Types...>::VariantWrapper(const VariantWrapper& other)
    : variant_(other.variant_) {}

template <typename... Types>
VariantWrapper<Types...>::VariantWrapper(VariantWrapper&& other) noexcept
    : variant_(std::move(other.variant_)) {}

template <typename... Types>
auto VariantWrapper<Types...>::operator=(const VariantWrapper& other)
    -> VariantWrapper& {
    if (this != &other) {
        variant_ = other.variant_;
    }
    return *this;
}

template <typename... Types>
auto VariantWrapper<Types...>::operator=(VariantWrapper&& other) noexcept
    -> VariantWrapper& {
    if (this != &other) {
        variant_ = std::move(other.variant_);
    }
    return *this;
}

template <typename... Types>
template <typename T>
auto VariantWrapper<Types...>::operator=(T&& value) -> VariantWrapper& {
    variant_ = std::forward<T>(value);
    return *this;
}

template <typename... Types>
auto VariantWrapper<Types...>::typeName() const -> std::string {
    return std::visit(
        [](auto&& arg) -> std::string { return typeid(arg).name(); }, variant_);
}

template <typename... Types>
template <typename T>
auto VariantWrapper<Types...>::get() const -> T {
    if (!std::holds_alternative<T>(variant_)) {
        throw std::bad_variant_access();
    }
    return std::get<T>(variant_);
}

template <typename... Types>
template <typename T>
auto VariantWrapper<Types...>::is() const -> bool {
    return std::holds_alternative<T>(variant_);
}

template <typename... Types>
void VariantWrapper<Types...>::print() const {
    std::visit(
        [](const auto& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                         std::monostate>) {
                std::cout << "Current value: std::monostate" << std::endl;
            } else {
                std::cout << "Current value: " << value << std::endl;
            }
        },
        variant_);
}

template <typename... Types>
auto VariantWrapper<Types...>::operator==(const VariantWrapper& other) const
    -> bool {
    return variant_ == other.variant_;
}

template <typename... Types>
auto VariantWrapper<Types...>::operator!=(const VariantWrapper& other) const
    -> bool {
    return !(*this == other);
}

template <typename... Types>
template <typename Visitor>
auto VariantWrapper<Types...>::visit(Visitor&& visitor) const
    -> decltype(auto) {
    return std::visit(std::forward<Visitor>(visitor), variant_);
}

template <typename... Types>
auto VariantWrapper<Types...>::index() const -> std::size_t {
    return variant_.index();
}

template <typename... Types>
template <typename T>
auto VariantWrapper<Types...>::tryGet() const -> std::optional<T> {
    if (is<T>()) {
        return get<T>();
    }
    return std::nullopt;
}

template <typename... Types>
auto VariantWrapper<Types...>::toInt() const -> std::optional<int> {
    return visit([](auto&& arg) -> std::optional<int> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_convertible_v<T, int>) {
            return static_cast<int>(arg);
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            try {
                return std::stoi(arg);
            } catch (...) {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    });
}

template <typename... Types>
auto VariantWrapper<Types...>::toDouble() const -> std::optional<double> {
    return visit([](auto&& arg) -> std::optional<double> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_convertible_v<T, double>) {
            return static_cast<double>(arg);
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            try {
                return std::stod(arg);
            } catch (...) {
                return std::nullopt;
            }
        } else {
            return std::nullopt;
        }
    });
}

template <typename... Types>
auto VariantWrapper<Types...>::toBool() const -> std::optional<bool> {
    return visit([](auto&& arg) -> std::optional<bool> {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_convertible_v<T, bool>) {
            return static_cast<bool>(arg);
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            if (arg == "true") {
                return true;
            }
            if (arg == "false") {
                return false;
            }
            return std::nullopt;
        } else {
            return std::nullopt;
        }
    });
}

template <typename... Types>
auto VariantWrapper<Types...>::toString() const -> std::string {
    return visit([](auto&& arg) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                     std::monostate>) {
            return "std::monostate";
        } else {
            std::ostringstream oss;
            oss << arg;
            return oss.str();
        }
    });
}

template <typename... Types>
void VariantWrapper<Types...>::reset() {
    variant_.template emplace<std::monostate>();
}

template <typename... Types>
auto VariantWrapper<Types...>::hasValue() const -> bool {
    return variant_.index() != 0;  // std::monostate is the first type
}

template <typename... Types>
auto operator<<(std::ostream& outputStream,
                const VariantWrapper<Types...>& variantWrapper)
    -> std::ostream& {
    variantWrapper.print();
    return outputStream;
}
}  // namespace atom::type

#endif