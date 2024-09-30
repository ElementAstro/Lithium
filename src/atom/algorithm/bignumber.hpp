#ifndef ATOM_ALGORITHM_BIGNUMBER_HPP
#define ATOM_ALGORITHM_BIGNUMBER_HPP

#include <string>

#include "macro.hpp"

namespace atom::algorithm {

/**
 * @class BigNumber
 * @brief A class to represent and manipulate large numbers.
 */
class BigNumber {
public:
    /**
     * @brief Constructs a BigNumber from a string.
     * @param number The string representation of the number.
     */
    BigNumber(std::string number) : numberString_(std::move(number)) {
        numberString_ = trimLeadingZeros().numberString_;
    }

    /**
     * @brief Constructs a BigNumber from a long long integer.
     * @param number The long long integer representation of the number.
     */
    BigNumber(long long number) : numberString_(std::to_string(number)) {}

    /**
     * @brief Adds two BigNumber objects.
     * @param other The other BigNumber to add.
     * @return The result of the addition.
     */
    ATOM_NODISCARD auto add(const BigNumber& other) const -> BigNumber;

    /**
     * @brief Subtracts one BigNumber from another.
     * @param other The other BigNumber to subtract.
     * @return The result of the subtraction.
     */
    ATOM_NODISCARD auto subtract(const BigNumber& other) const -> BigNumber;

    /**
     * @brief Multiplies two BigNumber objects.
     * @param other The other BigNumber to multiply.
     * @return The result of the multiplication.
     */
    ATOM_NODISCARD auto multiply(const BigNumber& other) const -> BigNumber;

    /**
     * @brief Divides one BigNumber by another.
     * @param other The other BigNumber to divide by.
     * @return The result of the division.
     */
    ATOM_NODISCARD auto divide(const BigNumber& other) const -> BigNumber;

    /**
     * @brief Raises the BigNumber to the power of an exponent.
     * @param exponent The exponent to raise the number to.
     * @return The result of the exponentiation.
     */
    ATOM_NODISCARD auto pow(int exponent) const -> BigNumber;

    /**
     * @brief Gets the string representation of the BigNumber.
     * @return The string representation of the number.
     */
    ATOM_NODISCARD auto getString() const -> std::string {
        return numberString_;
    }

    /**
     * @brief Sets the string representation of the BigNumber.
     * @param newStr The new string representation of the number.
     * @return A reference to the updated BigNumber.
     */
    auto setString(const std::string& newStr) -> BigNumber {
        numberString_ = newStr;
        return *this;
    }

    /**
     * @brief Negates the BigNumber.
     * @return The negated BigNumber.
     */
    ATOM_NODISCARD auto negate() const -> BigNumber {
        return numberString_[0] == '-' ? BigNumber(numberString_.substr(1))
                                       : BigNumber("-" + numberString_);
    }

    /**
     * @brief Trims leading zeros from the BigNumber.
     * @return The BigNumber with leading zeros removed.
     */
    ATOM_NODISCARD auto trimLeadingZeros() const -> BigNumber;

    /**
     * @brief Checks if two BigNumber objects are equal.
     * @param other The other BigNumber to compare with.
     * @return True if the numbers are equal, false otherwise.
     */
    ATOM_NODISCARD auto equals(const BigNumber& other) const -> bool {
        return numberString_ == other.numberString_;
    }

    /**
     * @brief Checks if the BigNumber is equal to a long long integer.
     * @param other The long long integer to compare with.
     * @return True if the number is equal to the integer, false otherwise.
     */
    ATOM_NODISCARD auto equals(const long long& other) const -> bool {
        return numberString_ == std::to_string(other);
    }

    /**
     * @brief Checks if the BigNumber is equal to a string.
     * @param other The string to compare with.
     * @return True if the number is equal to the string, false otherwise.
     */
    ATOM_NODISCARD auto equals(const std::string& other) const -> bool {
        return numberString_ == other;
    }

    /**
     * @brief Gets the number of digits in the BigNumber.
     * @return The number of digits.
     */
    ATOM_NODISCARD auto digits() const -> unsigned int {
        return numberString_.length() - static_cast<int>(isNegative());
    }

    /**
     * @brief Checks if the BigNumber is negative.
     * @return True if the number is negative, false otherwise.
     */
    ATOM_NODISCARD auto isNegative() const -> bool {
        return numberString_[0] == '-';
    }

    /**
     * @brief Checks if the BigNumber is positive.
     * @return True if the number is positive, false otherwise.
     */
    ATOM_NODISCARD auto isPositive() const -> bool { return !isNegative(); }

    /**
     * @brief Checks if the BigNumber is even.
     * @return True if the number is even, false otherwise.
     */
    ATOM_NODISCARD auto isEven() const -> bool {
        return (numberString_.back() - '0') % 2 == 0;
    }

    /**
     * @brief Checks if the BigNumber is odd.
     * @return True if the number is odd, false otherwise.
     */
    ATOM_NODISCARD auto isOdd() const -> bool { return !isEven(); }

    /**
     * @brief Gets the absolute value of the BigNumber.
     * @return The absolute value of the number.
     */
    ATOM_NODISCARD auto abs() const -> BigNumber {
        return isNegative() ? BigNumber(numberString_.substr(1)) : *this;
    }

    /**
     * @brief Overloads the stream insertion operator for BigNumber.
     * @param os The output stream.
     * @param num The BigNumber to insert into the stream.
     * @return The output stream.
     */
    friend auto operator<<(std::ostream& os,
                           const BigNumber& num) -> std::ostream& {
        os << num.numberString_;
        return os;
    }

    /**
     * @brief Overloads the addition operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return The result of the addition.
     */
    friend auto operator+(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.add(b2);
    }

    /**
     * @brief Overloads the subtraction operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return The result of the subtraction.
     */
    friend auto operator-(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.subtract(b2);
    }

    /**
     * @brief Overloads the multiplication operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return The result of the multiplication.
     */
    friend auto operator*(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.multiply(b2);
    }

    /**
     * @brief Overloads the division operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return The result of the division.
     */
    friend auto operator/(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.divide(b2);
    }

    /**
     * @brief Overloads the exponentiation operator for BigNumber.
     * @param b1 The BigNumber base.
     * @param b2 The exponent.
     * @return The result of the exponentiation.
     */
    friend auto operator^(const BigNumber& b1, int b2) -> BigNumber {
        return b1.pow(b2);
    }

    /**
     * @brief Overloads the equality operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return True if the numbers are equal, false otherwise.
     */
    friend auto operator==(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1.equals(b2);
    }

    /**
     * @brief Overloads the greater than operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return True if the first number is greater than the second, false
     * otherwise.
     */
    friend auto operator>(const BigNumber& b1, const BigNumber& b2) -> bool;

    /**
     * @brief Overloads the less than operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return True if the first number is less than the second, false
     * otherwise.
     */
    friend auto operator<(const BigNumber& b1, const BigNumber& b2) -> bool {
        return !(b1 == b2) && !(b1 > b2);
    }

    /**
     * @brief Overloads the greater than or equal to operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return True if the first number is greater than or equal to the second,
     * false otherwise.
     */
    friend auto operator>=(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1 > b2 || b1 == b2;
    }

    /**
     * @brief Overloads the less than or equal to operator for BigNumber.
     * @param b1 The first BigNumber.
     * @param b2 The second BigNumber.
     * @return True if the first number is less than or equal to the second,
     * false otherwise.
     */
    friend auto operator<=(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1 < b2 || b1 == b2;
    }

    /**
     * @brief Overloads the addition assignment operator for BigNumber.
     * @param other The other BigNumber to add.
     * @return A reference to the updated BigNumber.
     */
    auto operator+=(const BigNumber& other) -> BigNumber& {
        *this = *this + other;
        return *this;
    }

    /**
     * @brief Overloads the subtraction assignment operator for BigNumber.
     * @param other The other BigNumber to subtract.
     * @return A reference to the updated BigNumber.
     */
    auto operator-=(const BigNumber& other) -> BigNumber& {
        *this = *this - other;
        return *this;
    }

    /**
     * @brief Overloads the multiplication assignment operator for BigNumber.
     * @param other The other BigNumber to multiply.
     * @return A reference to the updated BigNumber.
     */
    auto operator*=(const BigNumber& other) -> BigNumber& {
        *this = *this * other;
        return *this;
    }

    /**
     * @brief Overloads the division assignment operator for BigNumber.
     * @param other The other BigNumber to divide by.
     * @return A reference to the updated BigNumber.
     */
    auto operator/=(const BigNumber& other) -> BigNumber& {
        *this = *this / other;
        return *this;
    }

    /**
     * @brief Overloads the prefix increment operator for BigNumber.
     * @return A reference to the incremented BigNumber.
     */
    auto operator++() -> BigNumber& {
        *this += BigNumber("1");
        return *this;
    }

    /**
     * @brief Overloads the prefix decrement operator for BigNumber.
     * @return A reference to the decremented BigNumber.
     */
    auto operator--() -> BigNumber& {
        *this -= BigNumber("1");
        return *this;
    }

    /**
     * @brief Overloads the postfix increment operator for BigNumber.
     * @return The BigNumber before incrementing.
     */
    auto operator++(int) -> BigNumber {
        BigNumber t(*this);
        ++(*this);
        return t;
    }

    /**
     * @brief Overloads the postfix decrement operator for BigNumber.
     * @return The BigNumber before decrementing.
     */
    auto operator--(int) -> BigNumber {
        BigNumber t(*this);
        --(*this);
        return t;
    }

    /**
     * @brief Overloads the subscript operator for BigNumber.
     * @param index The index of the digit to access.
     * @return The digit at the specified index.
     */
    auto operator[](int index) const -> unsigned int {
        return static_cast<unsigned int>(numberString_[index] - '0');
    }

private:
    std::string numberString_;  ///< The string representation of the number.
};

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_BIGNUMBER_HPP