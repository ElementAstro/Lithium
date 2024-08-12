#include "atom/algorithm/fraction.hpp"
#include <gtest/gtest.h>

// Tests for Fraction class
TEST(FractionTest, Constructor) {
    atom::algorithm::Fraction f1;
    EXPECT_EQ(f1.numerator, 0);
    EXPECT_EQ(f1.denominator, 1);

    atom::algorithm::Fraction f2(3, 4);
    EXPECT_EQ(f2.numerator, 3);
    EXPECT_EQ(f2.denominator, 4);

    atom::algorithm::Fraction f3(6, -8);
    EXPECT_EQ(f3.numerator, -3);
    EXPECT_EQ(f3.denominator, 4);
}

TEST(FractionTest, Addition) {
    atom::algorithm::Fraction f1(1, 2);
    atom::algorithm::Fraction f2(1, 3);
    atom::algorithm::Fraction result = f1 + f2;
    EXPECT_EQ(result.numerator, 5);
    EXPECT_EQ(result.denominator, 6);
}

TEST(FractionTest, Subtraction) {
    atom::algorithm::Fraction f1(1, 2);
    atom::algorithm::Fraction f2(1, 3);
    atom::algorithm::Fraction result = f1 - f2;
    EXPECT_EQ(result.numerator, 1);
    EXPECT_EQ(result.denominator, 6);
}

TEST(FractionTest, Multiplication) {
    atom::algorithm::Fraction f1(1, 2);
    atom::algorithm::Fraction f2(1, 3);
    atom::algorithm::Fraction result = f1 * f2;
    EXPECT_EQ(result.numerator, 1);
    EXPECT_EQ(result.denominator, 6);
}

TEST(FractionTest, Division) {
    atom::algorithm::Fraction f1(1, 2);
    atom::algorithm::Fraction f2(1, 3);
    atom::algorithm::Fraction result = f1 / f2;
    EXPECT_EQ(result.numerator, 3);
    EXPECT_EQ(result.denominator, 2);
}

TEST(FractionTest, Equality) {
    atom::algorithm::Fraction f1(1, 2);
    atom::algorithm::Fraction f2(1, 2);
    atom::algorithm::Fraction f3(2, 4);
    EXPECT_TRUE(f1 == f2);
    EXPECT_TRUE(f1 == f3);
}

TEST(FractionTest, Conversion) {
    atom::algorithm::Fraction f1(1, 2);
    EXPECT_DOUBLE_EQ(static_cast<double>(f1), 0.5);
    EXPECT_FLOAT_EQ(static_cast<float>(f1), 0.5f);
    EXPECT_EQ(static_cast<int>(f1), 0);
}

TEST(FractionTest, ToString) {
    atom::algorithm::Fraction f1(1, 2);
    EXPECT_EQ(f1.toString(), "1/2");

    atom::algorithm::Fraction f2(-3, 4);
    EXPECT_EQ(f2.toString(), "-3/4");
}

TEST(FractionTest, ToDouble) {
    atom::algorithm::Fraction f1(1, 2);
    EXPECT_DOUBLE_EQ(f1.toDouble(), 0.5);
}

TEST(FractionTest, IOStream) {
    atom::algorithm::Fraction f1(1, 2);
    std::stringstream ss;
    ss << f1;
    EXPECT_EQ(ss.str(), "1/2");

    atom::algorithm::Fraction f2;
    ss >> f2;
    EXPECT_EQ(f2.numerator, 1);
    EXPECT_EQ(f2.denominator, 2);
}
