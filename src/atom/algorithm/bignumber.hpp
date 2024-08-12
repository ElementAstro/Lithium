#ifndef ATOM_ALGORITHM_BIGNUMBER_HPP
#define ATOM_ALGORITHM_BIGNUMBER_HPP

#include <string>

#include "macro.hpp"

namespace atom::algorithm {
class BigNumber {
public:
    BigNumber(std::string number) : numberString_(std::move(number)) {
        numberString_ = trimLeadingZeros().numberString_;
    }
    BigNumber(long long number) : numberString_(std::to_string(number)) {}

    ATOM_NODISCARD auto add(const BigNumber& other) const -> BigNumber;
    ATOM_NODISCARD auto subtract(const BigNumber& other) const -> BigNumber;
    ATOM_NODISCARD auto multiply(const BigNumber& other) const -> BigNumber;
    ATOM_NODISCARD auto divide(const BigNumber& other) const -> BigNumber;
    ATOM_NODISCARD auto pow(int exponent) const -> BigNumber;

    ATOM_NODISCARD auto getString() const -> std::string {
        return numberString_;
    }
    auto setString(const std::string& newStr) -> BigNumber {
        numberString_ = newStr;
        return *this;
    }

    ATOM_NODISCARD auto negate() const -> BigNumber {
        return numberString_[0] == '-' ? BigNumber(numberString_.substr(1))
                                       : BigNumber("-" + numberString_);
    }
    ATOM_NODISCARD auto trimLeadingZeros() const -> BigNumber;

    ATOM_NODISCARD auto equals(const BigNumber& other) const -> bool {
        return numberString_ == other.numberString_;
    }
    ATOM_NODISCARD auto equals(const long long& other) const -> bool {
        return numberString_ == std::to_string(other);
    }
    ATOM_NODISCARD auto equals(const std::string& other) const -> bool {
        return numberString_ == other;
    }

    ATOM_NODISCARD auto digits() const -> unsigned int {
        return numberString_.length() - static_cast<int>(isNegative());
    }
    ATOM_NODISCARD auto isNegative() const -> bool {
        return numberString_[0] == '-';
    }
    ATOM_NODISCARD auto isPositive() const -> bool { return !isNegative(); }
    ATOM_NODISCARD auto isEven() const -> bool {
        return (numberString_.back() - '0') % 2 == 0;
    }
    ATOM_NODISCARD auto isOdd() const -> bool { return !isEven(); }
    ATOM_NODISCARD auto abs() const -> BigNumber {
        return isNegative() ? BigNumber(numberString_.substr(1)) : *this;
    }

    friend auto operator<<(std::ostream& os,
                           const BigNumber& num) -> std::ostream& {
        os << num.numberString_;
        return os;
    }
    friend auto operator+(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.add(b2);
    }
    friend auto operator-(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.subtract(b2);
    }
    friend auto operator*(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.multiply(b2);
    }
    friend auto operator/(const BigNumber& b1,
                          const BigNumber& b2) -> BigNumber {
        return b1.divide(b2);
    }
    friend auto operator^(const BigNumber& b1, int b2) -> BigNumber {
        return b1.pow(b2);
    }
    friend auto operator==(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1.equals(b2);
    }
    friend auto operator>(const BigNumber& b1, const BigNumber& b2) -> bool;
    friend auto operator<(const BigNumber& b1, const BigNumber& b2) -> bool {
        return !(b1 == b2) && !(b1 > b2);
    }
    friend auto operator>=(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1 > b2 || b1 == b2;
    }
    friend auto operator<=(const BigNumber& b1, const BigNumber& b2) -> bool {
        return b1 < b2 || b1 == b2;
    }

    auto operator+=(const BigNumber& other) -> BigNumber& {
        *this = *this + other;
        return *this;
    }
    auto operator-=(const BigNumber& other) -> BigNumber& {
        *this = *this - other;
        return *this;
    }
    auto operator*=(const BigNumber& other) -> BigNumber& {
        *this = *this * other;
        return *this;
    }
    auto operator/=(const BigNumber& other) -> BigNumber& {
        *this = *this / other;
        return *this;
    }
    auto operator++() -> BigNumber& {
        *this += BigNumber("1");
        return *this;
    }
    auto operator--() -> BigNumber& {
        *this -= BigNumber("1");
        return *this;
    }
    auto operator++(int) -> BigNumber {
        BigNumber t(*this);
        ++(*this);
        return t;
    }
    auto operator--(int) -> BigNumber {
        BigNumber t(*this);
        --(*this);
        return t;
    }
    auto operator[](int index) const -> unsigned int {
        return static_cast<unsigned int>(numberString_[index] - '0');
    }

private:
    std::string numberString_;
};
}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_BIGNUMBER_HPP
