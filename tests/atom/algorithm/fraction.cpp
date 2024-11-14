#include "atom/algorithm/fraction.hpp"
#include <gtest/gtest.h>

using namespace atom::algorithm;

class FractionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Construction Tests
TEST_F(FractionTest, DefaultConstructor) {
    Fraction f;
    EXPECT_EQ(f.toString(), "0/1");
}

TEST_F(FractionTest, ConstructorWithValues) {
    Fraction f(3, 4);
    EXPECT_EQ(f.toString(), "3/4");
}

TEST_F(FractionTest, ConstructorAutoReduces) {
    Fraction f(2, 4);
    EXPECT_EQ(f.toString(), "1/2");
}

TEST_F(FractionTest, ConstructorWithNegatives) {
    Fraction f1(-3, 4);
    Fraction f2(3, -4);
    Fraction f3(-3, -4);
    EXPECT_EQ(f1.toString(), "-3/4");
    EXPECT_EQ(f2.toString(), "-3/4");
    EXPECT_EQ(f3.toString(), "3/4");
}

TEST_F(FractionTest, ConstructorThrowsOnZeroDenominator) {
    EXPECT_THROW(Fraction(1, 0), FractionException);
}

// Arithmetic Tests
TEST_F(FractionTest, Addition) {
    Fraction f1(1, 2);
    Fraction f2(1, 3);
    Fraction result = f1 + f2;
    EXPECT_EQ(result.toString(), "5/6");
}

TEST_F(FractionTest, Subtraction) {
    Fraction f1(3, 4);
    Fraction f2(1, 4);
    Fraction result = f1 - f2;
    EXPECT_EQ(result.toString(), "1/2");
}

TEST_F(FractionTest, Multiplication) {
    Fraction f1(2, 3);
    Fraction f2(3, 4);
    Fraction result = f1 * f2;
    EXPECT_EQ(result.toString(), "1/2");
}

TEST_F(FractionTest, Division) {
    Fraction f1(1, 2);
    Fraction f2(1, 4);
    Fraction result = f1 / f2;
    EXPECT_EQ(result.toString(), "2/1");
}

// Compound Assignment Tests
TEST_F(FractionTest, CompoundAddition) {
    Fraction f(1, 2);
    f += Fraction(1, 4);
    EXPECT_EQ(f.toString(), "3/4");
}

TEST_F(FractionTest, CompoundSubtraction) {
    Fraction f(3, 4);
    f -= Fraction(1, 4);
    EXPECT_EQ(f.toString(), "1/2");
}

TEST_F(FractionTest, CompoundMultiplication) {
    Fraction f(2, 3);
    f *= Fraction(3, 4);
    EXPECT_EQ(f.toString(), "1/2");
}

TEST_F(FractionTest, CompoundDivision) {
    Fraction f(1, 2);
    f /= Fraction(2, 3);
    EXPECT_EQ(f.toString(), "3/4");
}

// Comparison Tests
TEST_F(FractionTest, Equality) {
    EXPECT_EQ(Fraction(1, 2), Fraction(2, 4));
    EXPECT_NE(Fraction(1, 2), Fraction(1, 3));
}

#if __cplusplus >= 202002L
TEST_F(FractionTest, ThreeWayComparison) {
    EXPECT_TRUE(Fraction(1, 2) < Fraction(2, 3));
    EXPECT_TRUE(Fraction(3, 4) > Fraction(1, 2));
    EXPECT_TRUE(Fraction(1, 2) <= Fraction(1, 2));
    EXPECT_TRUE(Fraction(1, 2) >= Fraction(1, 2));
}
#endif

// Conversion Tests
TEST_F(FractionTest, ToDouble) {
    Fraction f(1, 2);
    EXPECT_DOUBLE_EQ(static_cast<double>(f), 0.5);
}

TEST_F(FractionTest, ToFloat) {
    Fraction f(1, 4);
    EXPECT_FLOAT_EQ(static_cast<float>(f), 0.25f);
}

TEST_F(FractionTest, ToInt) {
    EXPECT_EQ(static_cast<int>(Fraction(5, 2)), 2);
    EXPECT_EQ(static_cast<int>(Fraction(-5, 2)), -2);
}

// Helper Method Tests
TEST_F(FractionTest, ToString) {
    EXPECT_EQ(Fraction(1, 2).toString(), "1/2");
    EXPECT_EQ(Fraction(-1, 2).toString(), "-1/2");
    EXPECT_EQ(Fraction(0, 1).toString(), "0/1");
}

TEST_F(FractionTest, Invert) {
    Fraction f(2, 3);
    f.invert();
    EXPECT_EQ(f.toString(), "3/2");
}

TEST_F(FractionTest, InvertThrowsOnZero) {
    Fraction f(0, 1);
    EXPECT_THROW(f.invert(), FractionException);
}

TEST_F(FractionTest, Abs) {
    EXPECT_EQ(Fraction(-1, 2).abs(), Fraction(1, 2));
    EXPECT_EQ(Fraction(1, 2).abs(), Fraction(1, 2));
}

TEST_F(FractionTest, IsZero) {
    EXPECT_TRUE(Fraction(0, 1).isZero());
    EXPECT_FALSE(Fraction(1, 2).isZero());
}

TEST_F(FractionTest, IsPositive) {
    EXPECT_TRUE(Fraction(1, 2).isPositive());
    EXPECT_FALSE(Fraction(-1, 2).isPositive());
    EXPECT_FALSE(Fraction(0, 1).isPositive());
}

TEST_F(FractionTest, IsNegative) {
    EXPECT_TRUE(Fraction(-1, 2).isNegative());
    EXPECT_FALSE(Fraction(1, 2).isNegative());
    EXPECT_FALSE(Fraction(0, 1).isNegative());
}

// Stream Operation Tests
TEST_F(FractionTest, StreamOutput) {
    std::ostringstream oss;
    oss << Fraction(1, 2);
    EXPECT_EQ(oss.str(), "1/2");
}

TEST_F(FractionTest, StreamInput) {
    std::istringstream iss("3/4");
    Fraction f;
    iss >> f;
    EXPECT_EQ(f, Fraction(3, 4));
}

// Factory Function Tests
TEST_F(FractionTest, MakeFractionFromInt) {
    auto f = makeFraction(5);
    EXPECT_EQ(f, Fraction(5, 1));
}

TEST_F(FractionTest, MakeFractionFromDouble) {
    auto f = makeFraction(0.5, 1000);
    EXPECT_EQ(f, Fraction(1, 2));
}

// Edge Cases and Error Tests
TEST_F(FractionTest, ArithmeticOverflow) {
    Fraction f1(std::numeric_limits<int>::max(), 1);
    Fraction f2(1, 1);
    EXPECT_THROW(f1 + f2, FractionException);
}

TEST_F(FractionTest, DivisionByZero) {
    Fraction f1(1, 2);
    Fraction f2(0, 1);
    EXPECT_THROW(f1 / f2, FractionException);
}