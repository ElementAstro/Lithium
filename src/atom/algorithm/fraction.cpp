#include "fraction.hpp"

#include <cmath>
#include <compare>
#include <numeric>

namespace Atom::Algorithm {

int Fraction::gcd(int a, int b) { return std::gcd(a, b); }

void Fraction::reduce() {
    int g = gcd(numerator, denominator);
    numerator /= g;
    denominator /= g;
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
    int n, d;
    char slash;
    // TODO: Fix this
    // is >> n >> slash >> d;
    f = Fraction(n, d);  // Assuming Fraction has a constructor that takes
                         // numerator and denominator
    return is;
}

}  // namespace Atom::Algorithm
