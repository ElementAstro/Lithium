#include "atom/extra/boost/math.hpp"

#include <gtest/gtest.h>

using namespace atom::extra::boost;

// SpecialFunctions Tests
TEST(SpecialFunctionsTest, BetaFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::beta(0.5, 0.5), 3.14159, 1e-5);
    EXPECT_THROW(SpecialFunctions<double>::beta(-1, 0.5), std::domain_error);
}

TEST(SpecialFunctionsTest, GammaFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::gamma(5), 24.0, 1e-5);
    EXPECT_THROW(SpecialFunctions<double>::gamma(-1), std::domain_error);
}

TEST(SpecialFunctionsTest, DigammaFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::digamma(5), 1.50612, 1e-5);
}

TEST(SpecialFunctionsTest, ErfFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::erf(1), 0.8427, 1e-4);
}

TEST(SpecialFunctionsTest, BesselJFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::besselJ(0, 1), 0.7652, 1e-4);
}

TEST(SpecialFunctionsTest, LegendrePFunction) {
    EXPECT_NEAR(SpecialFunctions<double>::legendreP(2, 0.5), -0.125, 1e-5);
}

// Statistics Tests
TEST(StatisticsTest, Mean) {
    std::vector<double> data = {1, 2, 3, 4, 5};
    EXPECT_NEAR(Statistics<double>::mean(data), 3.0, 1e-5);
}

TEST(StatisticsTest, Variance) {
    std::vector<double> data = {1, 2, 3, 4, 5};
    EXPECT_NEAR(Statistics<double>::variance(data), 2.5, 1e-5);
}

TEST(StatisticsTest, Skewness) {
    std::vector<double> data = {1, 2, 3, 4, 5};
    EXPECT_NEAR(Statistics<double>::skewness(data), 0.0, 1e-5);
}

TEST(StatisticsTest, Kurtosis) {
    std::vector<double> data = {1, 2, 3, 4, 5};
    EXPECT_NEAR(Statistics<double>::kurtosis(data), -1.3, 1e-5);
}

// Distributions Tests
TEST(DistributionsTest, NormalDistribution) {
    Distributions<double>::NormalDistribution dist(0, 1);
    EXPECT_NEAR(dist.pdf(0), 0.3989, 1e-4);
    EXPECT_NEAR(dist.cdf(0), 0.5, 1e-5);
}

TEST(DistributionsTest, StudentTDistribution) {
    Distributions<double>::StudentTDistribution dist(10);
    EXPECT_NEAR(dist.pdf(0), 0.3891, 1e-4);
    EXPECT_NEAR(dist.cdf(0), 0.5, 1e-5);
}

TEST(DistributionsTest, PoissonDistribution) {
    Distributions<double>::PoissonDistribution dist(3);
    EXPECT_NEAR(dist.pdf(2), 0.2240, 1e-4);
    EXPECT_NEAR(dist.cdf(2), 0.4232, 1e-4);
}

TEST(DistributionsTest, ExponentialDistribution) {
    Distributions<double>::ExponentialDistribution dist(1);
    EXPECT_NEAR(dist.pdf(1), 0.3679, 1e-4);
    EXPECT_NEAR(dist.cdf(1), 0.6321, 1e-4);
}

// NumericalIntegration Tests
TEST(NumericalIntegrationTest, Trapezoidal) {
    auto func = [](double x) { return x * x; };
    EXPECT_NEAR(NumericalIntegration<double>::trapezoidal(func, 0, 1),
                1.0 / 3.0, 1e-5);
}

// Optimization Tests
TEST(OptimizationTest, GoldenSectionSearch) {
    auto func = [](double x) { return (x - 2) * (x - 2); };
    EXPECT_NEAR(Optimization<double>::goldenSectionSearch(func, 0, 4, 1e-5),
                2.0, 1e-5);
}

TEST(OptimizationTest, NewtonRaphson) {
    auto func = [](double x) { return x * x - 2; };
    auto derivative = [](double x) { return 2 * x; };
    EXPECT_NEAR(
        Optimization<double>::newtonRaphson(func, derivative, 1.0, 1e-5, 100),
        std::sqrt(2), 1e-5);
}

// LinearAlgebra Tests
TEST(LinearAlgebraTest, SolveLinearSystem) {
    LinearAlgebra<double>::Matrix A(2, 2);
    A(0, 0) = 3;
    A(0, 1) = 2;
    A(1, 0) = 1;
    A(1, 1) = 2;
    LinearAlgebra<double>::Vector b(2);
    b(0) = 5;
    b(1) = 5;
    auto x = LinearAlgebra<double>::solveLinearSystem(A, b);
    EXPECT_NEAR(x(0), 1.0, 1e-5);
    EXPECT_NEAR(x(1), 2.0, 1e-5);
}

TEST(LinearAlgebraTest, Determinant) {
    LinearAlgebra<double>::Matrix A(2, 2);
    A(0, 0) = 3;
    A(0, 1) = 2;
    A(1, 0) = 1;
    A(1, 1) = 2;
    EXPECT_NEAR(LinearAlgebra<double>::determinant(A), 4.0, 1e-5);
}

TEST(LinearAlgebraTest, Multiply) {
    LinearAlgebra<double>::Matrix A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;
    LinearAlgebra<double>::Matrix B(2, 2);
    B(0, 0) = 2;
    B(0, 1) = 0;
    B(1, 0) = 1;
    B(1, 1) = 2;
    auto C = LinearAlgebra<double>::multiply(A, B);
    EXPECT_NEAR(C(0, 0), 4.0, 1e-5);
    EXPECT_NEAR(C(0, 1), 4.0, 1e-5);
    EXPECT_NEAR(C(1, 0), 10.0, 1e-5);
    EXPECT_NEAR(C(1, 1), 8.0, 1e-5);
}

TEST(LinearAlgebraTest, Transpose) {
    LinearAlgebra<double>::Matrix A(2, 2);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;
    auto B = LinearAlgebra<double>::transpose(A);
    EXPECT_NEAR(B(0, 0), 1.0, 1e-5);
    EXPECT_NEAR(B(0, 1), 3.0, 1e-5);
    EXPECT_NEAR(B(1, 0), 2.0, 1e-5);
    EXPECT_NEAR(B(1, 1), 4.0, 1e-5);
}

// ODESolver Tests
TEST(ODESolverTest, RungeKutta4) {
    auto system = [](const ODESolver<double>::State& x,
                     ODESolver<double>::State& dxdt, double t) {
        dxdt[0] = x[1];
        dxdt[1] = -x[0];
    };
    ODESolver<double>::State initialState = {1.0, 0.0};
    auto solution = ODESolver<double>::rungeKutta4(system, initialState, 0.0,
                                                   2 * M_PI, 0.1);
    EXPECT_NEAR(solution.back()[0], 1.0, 1e-1);
    EXPECT_NEAR(solution.back()[1], 0.0, 1e-1);
}

// FinancialMath Tests
TEST(FinancialMathTest, BlackScholesCall) {
    EXPECT_NEAR(FinancialMath<double>::blackScholesCall(100, 100, 0.05, 0.2, 1),
                10.4506, 1e-4);
}

TEST(FinancialMathTest, ModifiedDuration) {
    EXPECT_NEAR(FinancialMath<double>::modifiedDuration(0.05, 0.06, 1000, 10),
                7.7217, 1e-4);
}

TEST(FinancialMathTest, BondPrice) {
    EXPECT_NEAR(FinancialMath<double>::bondPrice(0.05, 0.06, 1000, 10), 1077.22,
                1e-2);
}

TEST(FinancialMathTest, ImpliedVolatility) {
    EXPECT_NEAR(
        FinancialMath<double>::impliedVolatility(10.4506, 100, 100, 0.05, 1),
        0.2, 1e-2);
}