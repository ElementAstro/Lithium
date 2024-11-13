/*
 * fraction.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-28

Description: Implementation of Fraction class

**************************************************/

#ifndef ATOM_ALGORITHM_FRACTION_HPP
#define ATOM_ALGORITHM_FRACTION_HPP

#include <cmath>
#include <compare>
#include <exception>
#include <functional>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

namespace atom::algorithm {

/**
 * @brief Exception class for Fraction errors.
 */
class FractionException : public std::runtime_error {
public:
    explicit FractionException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Represents a fraction with numerator and denominator.
 */
class Fraction {
private:
    int numerator;   /**< The numerator of the fraction. */
    int denominator; /**< The denominator of the fraction. */

    /**
     * @brief Computes the greatest common divisor (GCD) of two numbers.
     * @param a The first number.
     * @param b The second number.
     * @return The GCD of the two numbers.
     */
    static constexpr int gcd(int a, int b) noexcept;

    /**
     * @brief Reduces the fraction to its simplest form.
     */
    void reduce() noexcept;

public:
    /**
     * @brief Constructs a new Fraction object with the given numerator and
     * denominator.
     * @param n The numerator (default is 0).
     * @param d The denominator (default is 1).
     * @throws FractionException if the denominator is zero.
     */
    explicit constexpr Fraction(int n = 0, int d = 1);

    /**
     * @brief Adds another fraction to this fraction.
     * @param other The fraction to add.
     * @return Reference to the modified fraction.
     * @throws FractionException on arithmetic overflow.
     */
    auto operator+=(const Fraction& other) -> Fraction&;

    /**
     * @brief Subtracts another fraction from this fraction.
     * @param other The fraction to subtract.
     * @return Reference to the modified fraction.
     * @throws FractionException on arithmetic overflow.
     */
    auto operator-=(const Fraction& other) -> Fraction&;

    /**
     * @brief Multiplies this fraction by another fraction.
     * @param other The fraction to multiply by.
     * @return Reference to the modified fraction.
     * @throws FractionException if multiplication leads to zero denominator.
     */
    auto operator*=(const Fraction& other) -> Fraction&;

    /**
     * @brief Divides this fraction by another fraction.
     * @param other The fraction to divide by.
     * @return Reference to the modified fraction.
     * @throws FractionException if division by zero occurs.
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
     * @return A std::strong_ordering indicating the comparison result.
     */
    auto operator<=>(const Fraction& other) const -> std::strong_ordering;
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
     * @return The fraction as an integer (truncates towards zero).
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
     * @brief Inverts the fraction (reciprocal).
     * @return Reference to the modified fraction.
     * @throws FractionException if numerator is zero.
     */
    auto invert() -> Fraction&;

    /**
     * @brief Returns the absolute value of the fraction.
     * @return A new Fraction representing the absolute value.
     */
    [[nodiscard]] auto abs() const -> Fraction;

    /**
     * @brief Checks if the fraction is zero.
     * @return True if the fraction is zero, false otherwise.
     */
    [[nodiscard]] auto isZero() const -> bool;

    /**
     * @brief Checks if the fraction is positive.
     * @return True if the fraction is positive, false otherwise.
     */
    [[nodiscard]] auto isPositive() const -> bool;

    /**
     * @brief Checks if the fraction is negative.
     * @return True if the fraction is negative, false otherwise.
     */
    [[nodiscard]] auto isNegative() const -> bool;

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
     * @throws FractionException if the input format is invalid or denominator
     * is zero.
     */
    friend auto operator>>(std::istream& is, Fraction& f) -> std::istream&;
};

/**
 * @brief Creates a Fraction from an integer.
 * @param value The integer value.
 * @return A Fraction representing the integer.
 */
inline auto makeFraction(int value) -> Fraction;

/**
 * @brief Creates a Fraction from a double by approximating it.
 * @param value The double value.
 * @param max_denominator The maximum allowed denominator to limit the
 * approximation.
 * @return A Fraction approximating the double value.
 */
inline auto makeFraction(double value,
                         int max_denominator = 1000000) -> Fraction;

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_FRACTION_HPP