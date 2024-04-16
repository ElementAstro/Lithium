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
/**
 * @brief Represents a fraction with numerator and denominator.
 */
class Fraction {
    /**
     * @brief Computes the greatest common divisor (GCD) of two numbers.
     * @param a The first number.
     * @param b The second number.
     * @return The GCD of the two numbers.
     */
    static int gcd(int a, int b);

    /**
     * @brief Reduces the fraction to its simplest form.
     */
    void reduce();

    // For pybind11 compatibility
public:
    int numerator;   /**< The numerator of the fraction. */
    int denominator; /**< The denominator of the fraction. */

public:
    /**
     * @brief Constructs a new Fraction object with the given numerator and
     * denominator.
     * @param n The numerator (default is 0).
     * @param d The denominator (default is 1).
     */
    Fraction(int n = 0, int d = 1);

    /**
     * @brief Adds another fraction to this fraction.
     * @param other The fraction to add.
     * @return Reference to the modified fraction.
     */
    Fraction& operator+=(const Fraction& other);

    /**
     * @brief Subtracts another fraction from this fraction.
     * @param other The fraction to subtract.
     * @return Reference to the modified fraction.
     */
    Fraction& operator-=(const Fraction& other);

    /**
     * @brief Multiplies this fraction by another fraction.
     * @param other The fraction to multiply by.
     * @return Reference to the modified fraction.
     */
    Fraction& operator*=(const Fraction& other);

    /**
     * @brief Divides this fraction by another fraction.
     * @param other The fraction to divide by.
     * @return Reference to the modified fraction.
     */
    Fraction& operator/=(const Fraction& other);

    /**
     * @brief Adds another fraction to this fraction.
     * @param other The fraction to add.
     * @return The result of addition.
     */
    Fraction operator+(const Fraction& other) const;

    /**
     * @brief Subtracts another fraction from this fraction.
     * @param other The fraction to subtract.
     * @return The result of subtraction.
     */
    Fraction operator-(const Fraction& other) const;

    /**
     * @brief Multiplies this fraction by another fraction.
     * @param other The fraction to multiply by.
     * @return The result of multiplication.
     */
    Fraction operator*(const Fraction& other) const;

    /**
     * @brief Divides this fraction by another fraction.
     * @param other The fraction to divide by.
     * @return The result of division.
     */
    Fraction operator/(const Fraction& other) const;

#if __cplusplus >= 202002L
    /**
     * @brief Compares this fraction with another fraction.
     * @param other The fraction to compare with.
     * @return An integer indicating the comparison result.
     */
    auto operator<=>(const Fraction& other) const {
        double diff = this->to_double() - other.to_double();
        if (diff > 0)
            return std::strong_ordering::greater;
        else if (diff < 0)
            return std::strong_ordering::less;
        else
            return std::strong_ordering::equal;
    }
#endif

    /**
     * @brief Checks if this fraction is equal to another fraction.
     * @param other The fraction to compare with.
     * @return True if fractions are equal, false otherwise.
     */
    bool operator==(const Fraction& other) const;

    /**
     * @brief Pre-increments the fraction by 1.
     * @return Reference to the modified fraction.
     */
    Fraction& operator++();

    /**
     * @brief Post-increments the fraction by 1.
     * @return The fraction before incrementing.
     */
    Fraction operator++(int);

    /**
     * @brief Pre-decrements the fraction by 1.
     * @return Reference to the modified fraction.
     */
    Fraction& operator--();

    /**
     * @brief Post-decrements the fraction by 1.
     * @return The fraction before decrementing.
     */
    Fraction operator--(int);

    /**
     * @brief Negates the fraction.
     * @return The negated fraction.
     */
    Fraction operator-() const;

    /**
     * @brief Converts the fraction to a double value.
     * @return The fraction as a double.
     */
    explicit operator double() const;

    /**
     * @brief Converts the fraction to a float value.
     * @return The fraction as a float.
     */
    explicit operator float() const;

    /**
     * @brief Converts the fraction to an integer value.
     * @return The fraction as an integer.
     */
    explicit operator int() const;

    /**
     * @brief Converts the fraction to a string representation.
     * @return The string representation of the fraction.
     */
    std::string to_string() const;

    /**
     * @brief Converts the fraction to a double value.
     * @return The fraction as a double.
     */
    double to_double() const;

    /**
     * @brief Outputs the fraction to the output stream.
     * @param os The output stream.
     * @param f The fraction to output.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Fraction& f);

    /**
     * @brief Inputs the fraction from the input stream.
     * @param is The input stream.
     * @param f The fraction to input.
     * @return Reference to the input stream.
     */
    friend std::istream& operator>>(std::istream& is, Fraction& f);
};

}  // namespace Atom::Algorithm

#endif
