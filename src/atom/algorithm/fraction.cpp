/*
 * fraction.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-28

Description: Implementation of Fraction class

**************************************************/

#include "fraction.hpp"

#include <numeric>
#include <sstream>

namespace atom::algorithm {

/* ------------------------ Private Methods ------------------------ */

constexpr int Fraction::gcd(int a, int b) noexcept {
    return (std::numeric_limits<int>::min() != a &&
            std::numeric_limits<int>::min() != b)
               ? std::abs(std::gcd(a, b))
               : 1;  // Prevent undefined behavior for min int
}

void Fraction::reduce() noexcept {
    if (denominator == 0) {
        // Denominator check is handled in constructors and operators
        return;
    }
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
    int divisor = gcd(numerator, denominator);
    numerator /= divisor;
    denominator /= divisor;
}

/* ------------------------ Arithmetic Operators ------------------------ */

auto Fraction::operator+=(const Fraction& other) -> Fraction& {
    // Avoid overflow by using long long for intermediate calculations
    long long commonDenominator =
        static_cast<long long>(denominator) * other.denominator;
    long long newNumerator =
        static_cast<long long>(numerator) * other.denominator +
        static_cast<long long>(other.numerator) * denominator;

    if (newNumerator > std::numeric_limits<int>::max() ||
        newNumerator < std::numeric_limits<int>::min() ||
        commonDenominator > std::numeric_limits<int>::max() ||
        commonDenominator < std::numeric_limits<int>::min()) {
        throw FractionException("Integer overflow during addition.");
    }

    numerator = static_cast<int>(newNumerator);
    denominator = static_cast<int>(commonDenominator);
    reduce();
    return *this;
}

auto Fraction::operator-=(const Fraction& other) -> Fraction& {
    long long commonDenominator =
        static_cast<long long>(denominator) * other.denominator;
    long long newNumerator =
        static_cast<long long>(numerator) * other.denominator -
        static_cast<long long>(other.numerator) * denominator;

    if (newNumerator > std::numeric_limits<int>::max() ||
        newNumerator < std::numeric_limits<int>::min() ||
        commonDenominator > std::numeric_limits<int>::max() ||
        commonDenominator < std::numeric_limits<int>::min()) {
        throw FractionException("Integer overflow during subtraction.");
    }

    numerator = static_cast<int>(newNumerator);
    denominator = static_cast<int>(commonDenominator);
    reduce();
    return *this;
}

auto Fraction::operator*=(const Fraction& other) -> Fraction& {
    if (other.numerator == 0) {
        numerator = 0;
        denominator = 1;
        return *this;
    }

    long long newNumerator =
        static_cast<long long>(numerator) * other.numerator;
    long long newDenominator =
        static_cast<long long>(denominator) * other.denominator;

    if (newNumerator > std::numeric_limits<int>::max() ||
        newNumerator < std::numeric_limits<int>::min() ||
        newDenominator > std::numeric_limits<int>::max() ||
        newDenominator < std::numeric_limits<int>::min()) {
        throw FractionException("Integer overflow during multiplication.");
    }

    numerator = static_cast<int>(newNumerator);
    denominator = static_cast<int>(newDenominator);
    reduce();
    return *this;
}

auto Fraction::operator/=(const Fraction& other) -> Fraction& {
    if (other.numerator == 0) {
        throw FractionException("Division by zero.");
    }

    long long newNumerator =
        static_cast<long long>(numerator) * other.denominator;
    long long newDenominator =
        static_cast<long long>(denominator) * other.numerator;

    if (newDenominator == 0) {
        throw FractionException("Denominator cannot be zero after division.");
    }

    if (newNumerator > std::numeric_limits<int>::max() ||
        newNumerator < std::numeric_limits<int>::min() ||
        newDenominator > std::numeric_limits<int>::max() ||
        newDenominator < std::numeric_limits<int>::min()) {
        throw FractionException("Integer overflow during division.");
    }

    numerator = static_cast<int>(newNumerator);
    denominator = static_cast<int>(newDenominator);
    if (denominator < 0) {  // Handle negative denominators
        numerator = -numerator;
        denominator = -denominator;
    }
    reduce();
    return *this;
}

/* ------------------------ Arithmetic Operators (Non-Member)
 * ------------------------ */

auto Fraction::operator+(const Fraction& other) const -> Fraction {
    Fraction result(*this);
    result += other;
    return result;
}

auto Fraction::operator-(const Fraction& other) const -> Fraction {
    Fraction result(*this);
    result -= other;
    return result;
}

auto Fraction::operator*(const Fraction& other) const -> Fraction {
    Fraction result(*this);
    result *= other;
    return result;
}

auto Fraction::operator/(const Fraction& other) const -> Fraction {
    Fraction result(*this);
    result /= other;
    return result;
}

/* ------------------------ Comparison Operators ------------------------ */

#if __cplusplus >= 202002L
auto Fraction::operator<=>(const Fraction& other) const
    -> std::strong_ordering {
    long long lhs = static_cast<long long>(numerator) * other.denominator;
    long long rhs = static_cast<long long>(other.numerator) * denominator;
    if (lhs < rhs) {
        return std::strong_ordering::less;
    }
    if (lhs > rhs) {
        return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
}
#endif

auto Fraction::operator==(const Fraction& other) const -> bool {
#if __cplusplus >= 202002L
    return (*this <=> other) == std::strong_ordering::equal;
#else
    return (numerator == other.numerator) && (denominator == other.denominator);
#endif
}

/* ------------------------ Type Conversion Operators ------------------------
 */

Fraction::operator double() const {
    return static_cast<double>(numerator) / denominator;
}

Fraction::operator float() const {
    return static_cast<float>(numerator) / denominator;
}

Fraction::operator int() const { return numerator / denominator; }

/* ------------------------ Utility Methods ------------------------ */

auto Fraction::toString() const -> std::string {
    std::ostringstream oss;
    oss << numerator << '/' << denominator;
    return oss.str();
}

auto Fraction::toDouble() const -> double { return static_cast<double>(*this); }

auto Fraction::invert() -> Fraction& {
    if (numerator == 0) {
        throw FractionException(
            "Cannot invert a fraction with numerator zero.");
    }
    std::swap(numerator, denominator);
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
    reduce();
    return *this;
}

auto Fraction::abs() const -> Fraction {
    return Fraction(numerator < 0 ? -numerator : numerator, denominator);
}

auto Fraction::isZero() const -> bool { return numerator == 0; }

auto Fraction::isPositive() const -> bool { return numerator > 0; }

auto Fraction::isNegative() const -> bool { return numerator < 0; }

/* ------------------------ Friend Functions ------------------------ */

auto operator<<(std::ostream& os, const Fraction& f) -> std::ostream& {
    os << f.toString();
    return os;
}

auto operator>>(std::istream& is, Fraction& f) -> std::istream& {
    int n = 0, d = 1;
    char sep = '/';
    is >> n >> sep >> d;
    if (sep != '/') {
        is.setstate(std::ios::failbit);
        throw FractionException(
            "Invalid input format. Expected 'numerator/denominator'.");
    }
    if (d == 0) {
        throw FractionException("Denominator cannot be zero.");
    }
    f.numerator = n;
    f.denominator = d;
    f.reduce();
    return is;
}

/* ------------------------ Inline Utility Functions ------------------------ */

auto makeFraction(int value) -> Fraction { return Fraction(value, 1); }

auto makeFraction(double value, int max_denominator) -> Fraction {
    if (std::isnan(value) || std::isinf(value)) {
        throw FractionException("Cannot create Fraction from NaN or Infinity.");
    }

    int sign = (value < 0) ? -1 : 1;
    value = std::abs(value);
    int numerator = 0;
    int denominator = 1;
    double minError = std::numeric_limits<double>::max();

    for (denominator = 1; denominator <= max_denominator; ++denominator) {
        numerator = static_cast<int>(std::round(value * denominator));
        double currentError =
            std::abs(value - static_cast<double>(numerator) / denominator);
        if (currentError < minError) {
            minError = currentError;
        } else {
            break;
        }
    }

    return Fraction(sign * numerator, denominator);
}

}  // namespace atom::algorithm