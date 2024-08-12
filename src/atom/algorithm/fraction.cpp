#include "fraction.hpp"

#include <cmath>
#include <numeric>
#include <sstream>

#include "atom/error/exception.hpp"

namespace atom::algorithm {

int Fraction::gcd(int a, int b) { return std::gcd(a, b); }

void Fraction::reduce() {
    int g = gcd(numerator, denominator);
    numerator /= g;
    denominator /= g;
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
}

Fraction::Fraction(int n, int d) : numerator(n), denominator(d) {
    if (denominator == 0) {
        THROW_INVALID_ARGUMENT("Denominator cannot be zero.");
    }
    reduce();
}

auto Fraction::operator+=(const Fraction &other) -> Fraction & {
    numerator = numerator * other.denominator + other.numerator * denominator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

auto Fraction::operator-=(const Fraction &other) -> Fraction & {
    numerator = numerator * other.denominator - other.numerator * denominator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

auto Fraction::operator*=(const Fraction &other) -> Fraction & {
    numerator *= other.numerator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

auto Fraction::operator/=(const Fraction &other) -> Fraction & {
    if (other.numerator == 0) {
        THROW_INVALID_ARGUMENT("Division by zero.");
    }
    numerator *= other.denominator;
    denominator *= other.numerator;
    reduce();
    return *this;
}

auto Fraction::operator+(const Fraction &other) const -> Fraction {
    Fraction result = *this;
    result += other;
    return result;
}

auto Fraction::operator-(const Fraction &other) const -> Fraction {
    Fraction result = *this;
    result -= other;
    return result;
}

auto Fraction::operator*(const Fraction &other) const -> Fraction {
    Fraction result = *this;
    result *= other;
    return result;
}

auto Fraction::operator/(const Fraction &other) const -> Fraction {
    Fraction result = *this;
    result /= other;
    return result;
}

auto Fraction::operator==(const Fraction &other) const -> bool {
    return (numerator == other.numerator) && (denominator == other.denominator);
}

Fraction::operator double() const { return toDouble(); }

Fraction::operator float() const { return static_cast<float>(toDouble()); }

Fraction::operator int() const { return numerator / denominator; }

auto Fraction::toString() const -> std::string {
    if (denominator == 1) {
        return std::to_string(numerator);
    }
    return std::to_string(numerator) + "/" + std::to_string(denominator);
}

auto Fraction::toDouble() const -> double {
    return static_cast<double>(numerator) / denominator;
}

auto operator<<(std::ostream &os, const Fraction &f) -> std::ostream & {
    os << f.toString();
    return os;
}

auto operator>>(std::istream &is, Fraction &f) -> std::istream & {
    std::string input;
    is >> input;
    std::istringstream iss(input);

    if (input.find('/') != std::string::npos) {
        int n, d;
        char slash;
        if (iss >> n >> slash >> d && slash == '/') {
            if (d == 0) {
                is.setstate(std::ios::failbit);
            } else {
                f = Fraction(n, d);
            }
        } else {
            is.setstate(std::ios::failbit);
        }
    } else {
        double value;
        if (iss >> value) {
            int n = static_cast<int>(value * 10000);
            int d = 10000;
            f = Fraction(n, d);
        } else {
            is.setstate(std::ios::failbit);
        }
    }

    return is;
}

}  // namespace atom::algorithm
