#include <gtest/gtest.h>
#include "atom/algorithm/math.hpp"

TEST(FractionTest, Addition)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    Atom::Algorithm::Fraction result = f1 + f2;
    EXPECT_EQ(result.getNumerator(), 5);
    EXPECT_EQ(result.getDenominator(), 6);
}

TEST(FractionTest, Subtraction)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    Atom::Algorithm::Fraction result = f1 - f2;
    EXPECT_EQ(result.getNumerator(), 1);
    EXPECT_EQ(result.getDenominator(), 6);
}

TEST(FractionTest, Multiplication)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    Atom::Algorithm::Fraction result = f1 * f2;
    EXPECT_EQ(result.getNumerator(), 1);
    EXPECT_EQ(result.getDenominator(), 6);
}

TEST(FractionTest, Division)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    Atom::Algorithm::Fraction result = f1 / f2;
    EXPECT_EQ(result.getNumerator(), 3);
    EXPECT_EQ(result.getDenominator(), 2);
}

TEST(FractionTest, Equality)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 2);
    EXPECT_TRUE(f1 == f2);
    EXPECT_FALSE(f1 != f2);
}

TEST(FractionTest, Inequality)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    EXPECT_TRUE(f1 != f2);
    EXPECT_FALSE(f1 == f2);
}

TEST(FractionTest, GreaterThan)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    EXPECT_TRUE(f1 > f2);
    EXPECT_FALSE(f1 < f2);
}

TEST(FractionTest, GreaterThanOrEqual)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    EXPECT_TRUE(f1 >= f2);
    EXPECT_FALSE(f1 <= f2);
}

TEST(FractionTest, LessThan)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    EXPECT_FALSE(f1 < f2);
    EXPECT_TRUE(f1 > f2);
}

TEST(FractionTest, LessThanOrEqual)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    EXPECT_FALSE(f1 <= f2);
    EXPECT_TRUE(f1 >= f2);
}

TEST(FractionTest, InputOutput)
{
    Atom::Algorithm::Fraction f1(1, 2);
    std::stringstream output;
    output << f1;
    EXPECT_EQ(output.str(), "1/2");
}

TEST(FractionTest, Assignment)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    f1 = f2;
    EXPECT_EQ(f1.getNumerator(), 1);
    EXPECT_EQ(f1.getDenominator(), 3);
}

TEST(FractionTest, Negative)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction result = -f1;
    EXPECT_EQ(result.getNumerator(), -1);
    EXPECT_EQ(result.getDenominator(), 2);
}

TEST(FractionTest, AdditionAssignment)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    f1 += f2;
    EXPECT_EQ(f1.getNumerator(), 5);
    EXPECT_EQ(f1.getDenominator(), 6);
}

TEST(FractionTest, SubtractionAssignment)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    f1 -= f2;
    EXPECT_EQ(f1.getNumerator(), 1);
    EXPECT_EQ(f1.getDenominator(), 6);
}

TEST(FractionTest, MultiplicationAssignment)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    f1 *= f2;
    EXPECT_EQ(f1.getNumerator(), 1);
    EXPECT_EQ(f1.getDenominator(), 6);
}

TEST(FractionTest, DivisionAssignment)
{
    Atom::Algorithm::Fraction f1(1, 2);
    Atom::Algorithm::Fraction f2(1, 3);
    f1 /= f2;
    EXPECT_EQ(f1.getNumerator(), 3);
    EXPECT_EQ(f1.getDenominator(), 2);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
