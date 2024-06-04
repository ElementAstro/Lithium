#include "fraction.hpp"

#include <cmath>
#include <compare>
#include <numeric>
#include <sstream>

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
        throw std::invalid_argument("Denominator cannot be zero.");
    }
    reduce();
}

Fraction &Fraction::operator+=(const Fraction &other) {
    numerator = numerator * other.denominator + other.numerator * denominator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

Fraction &Fraction::operator-=(const Fraction &other) {
    numerator = numerator * other.denominator - other.numerator * denominator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

Fraction &Fraction::operator*=(const Fraction &other) {
    numerator *= other.numerator;
    denominator *= other.denominator;
    reduce();
    return *this;
}

Fraction &Fraction::operator/=(const Fraction &other) {
    if (other.numerator == 0) {
        throw std::invalid_argument("Division by zero.");
    }
    numerator *= other.denominator;
    denominator *= other.numerator;
    reduce();
    return *this;
}

Fraction Fraction::operator+(const Fraction &other) const {
    Fraction result = *this;
    result += other;
    return result;
}

Fraction Fraction::operator-(const Fraction &other) const {
    Fraction result = *this;
    result -= other;
    return result;
}

Fraction Fraction::operator*(const Fraction &other) const {
    Fraction result = *this;
    result *= other;
    return result;
}

Fraction Fraction::operator/(const Fraction &other) const {
    Fraction result = *this;
    result /= other;
    return result;
}

bool Fraction::operator==(const Fraction &other) const {
    return (numerator == other.numerator) && (denominator == other.denominator);
}

Fraction::operator double() const { return to_double(); }

Fraction::operator float() const { return static_cast<float>(to_double()); }

Fraction::operator int() const { return numerator / denominator; }

std::string Fraction::to_string() const {
    if (denominator == 1) {
        return std::to_string(numerator);
    }
    return std::to_string(numerator) + "/" + std::to_string(denominator);
}

double Fraction::to_double() const {
    return static_cast<double>(numerator) / denominator;
}

std::ostream &operator<<(std::ostream &os, const Fraction &f) {
    os << f.to_string();
    return os;
}

std::istream &operator>>(std::istream &is, Fraction &f) {
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
