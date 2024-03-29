/*
 * fraction.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-28

Description: Implementation of fraction class

**************************************************/

#ifndef ATOM_ALGORITHM_FRACTION_HPP
#define ATOM_ALGORITHM_FRACTION_HPP

#include <compare>
#include <ostream>
#include <string>


namespace Atom::Algorithm {

class Fraction {
private:
    int numerator;
    int denominator;

    static int gcd(int a, int b);
    void reduce();

public:
    Fraction(int n = 0, int d = 1);
    Fraction& operator+=(const Fraction& other);
    Fraction& operator-=(const Fraction& other);
    Fraction& operator*=(const Fraction& other);
    Fraction& operator/=(const Fraction& other);
    Fraction operator+(const Fraction& other) const;
    Fraction operator-(const Fraction& other) const;
    Fraction operator*(const Fraction& other) const;
    Fraction operator/(const Fraction& other) const;
    auto operator<=>(const Fraction& other) const;
    bool operator==(const Fraction& other) const;
    Fraction& operator++();
    Fraction operator++(int);
    Fraction& operator--();
    Fraction operator--(int);
    Fraction operator-() const;
    explicit operator double() const;
    explicit operator float() const;
    explicit operator int() const;
    std::string to_string() const;
    double to_double() const;
    friend std::ostream& operator<<(std::ostream& os, const Fraction& f);
    friend std::istream& operator>>(std::istream& is, Fraction& f);
};

}  // namespace Atom::Algorithm

#endif
