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

namespace atom::algorithm {
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

    /**
     * @brief Constructs a new Fraction object with the given numerator and
     * denominator.
     * @param n The numerator (default is 0).
     * @param d The denominator (default is 1).
     */
    explicit Fraction(int n = 0, int d = 1);

    /**
     * @brief Adds another fraction to this fraction.
     * @param other The fraction to add.
     * @return Reference to the modified fraction.
     */
    auto operator+=(const Fraction& other) -> Fraction&;

    /**
     * @brief Subtracts another fraction from this fraction.
     * @param other The fraction to subtract.
     * @return Reference to the modified fraction.
     */
    auto operator-=(const Fraction& other) -> Fraction&;

    /**
     * @brief Multiplies this fraction by another fraction.
     * @param other The fraction to multiply by.
     * @return Reference to the modified fraction.
     */
    auto operator*=(const Fraction& other) -> Fraction&;

    /**
     * @brief Divides this fraction by another fraction.
     * @param other The fraction to divide by.
     * @return Reference to the modified fraction.
     */
    auto operator/=(const Fraction& other) -> Fraction&;

    /**
     * @brief Adds another fraction to this fraction.
     * @param other The fraction to add.
     * @return The result of addition.
     */
    auto operator+(const Fraction& other) const -> Fraction;

    /**
     * @brief Subtracts another fraction from this fraction.
     * @param other The fraction to subtract.
     * @return The result of subtraction.
     */
    auto operator-(const Fraction& other) const -> Fraction;

    /**
     * @brief Multiplies this fraction by another fraction.
     * @param other The fraction to multiply by.
     * @return The result of multiplication.
     */
    auto operator*(const Fraction& other) const -> Fraction;

    /**
     * @brief Divides this fraction by another fraction.
     * @param other The fraction to divide by.
     * @return The result of division.
     */
    auto operator/(const Fraction& other) const -> Fraction;

#if __cplusplus >= 202002L
    /**
     * @brief Compares this fraction with another fraction.
     * @param other The fraction to compare with.
     * @return An integer indicating the comparison result.
     */
    auto operator<=>(const Fraction& other) const {
        double diff = this->toDouble() - other.toDouble();
        if (diff > 0) {
            return std::strong_ordering::greater;
        }
        if (diff < 0) {
            return std::strong_ordering::less;
        }
        return std::strong_ordering::equal;
    }
#endif

    /**
     * @brief Checks if this fraction is equal to another fraction.
     * @param other The fraction to compare with.
     * @return True if fractions are equal, false otherwise.
     */
    auto operator==(const Fraction& other) const -> bool;

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
    [[nodiscard]] auto toString() const -> std::string;

    /**
     * @brief Converts the fraction to a double value.
     * @return The fraction as a double.
     */
    [[nodiscard]] auto toDouble() const -> double;

    /**
     * @brief Outputs the fraction to the output stream.
     * @param os The output stream.
     * @param f The fraction to output.
     * @return Reference to the output stream.
     */
    friend auto operator<<(std::ostream& os,
                           const Fraction& f) -> std::ostream&;

    /**
     * @brief Inputs the fraction from the input stream.
     * @param is The input stream.
     * @param f The fraction to input.
     * @return Reference to the input stream.
     */
    friend auto operator>>(std::istream& is, Fraction& f) -> std::istream&;
};

}  // namespace atom::algorithm

#endif
