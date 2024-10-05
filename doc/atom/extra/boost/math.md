# atom::extra::boost Namespace Documentation

## Overview

The `atom::extra::boost` namespace provides a collection of mathematical and statistical utilities built on top of the Boost C++ libraries. It includes classes for special functions, statistics, probability distributions, numerical integration, optimization, linear algebra, ODE solving, and financial mathematics.

## Table of Contents

1. [SpecialFunctions](#specialfunctions)
2. [Statistics](#statistics)
3. [Distributions](#distributions)
4. [NumericalIntegration](#numericalintegration)
5. [Optimization](#optimization)
6. [LinearAlgebra](#linearalgebra)
7. [ODESolver](#odesolver)
8. [FinancialMath](#financialmath)
9. [Utility Functions](#utility-functions)

## SpecialFunctions

The `SpecialFunctions` class provides static methods for various mathematical special functions.

### Methods

```cpp
template <Numeric T>
class SpecialFunctions {
public:
    static auto beta(T alpha, T beta) -> T;
    static auto gamma(T value) -> T;
    static auto digamma(T value) -> T;
    static auto erf(T value) -> T;
    static auto besselJ(int order, T value) -> T;
    static auto legendreP(int order, T value) -> T;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>

int main() {
    double alpha = 2.0, beta = 3.0;
    std::cout << "Beta(" << alpha << ", " << beta << ") = "
              << atom::extra::boost::SpecialFunctions<double>::beta(alpha, beta) << std::endl;

    double x = 1.5;
    std::cout << "Gamma(" << x << ") = "
              << atom::extra::boost::SpecialFunctions<double>::gamma(x) << std::endl;

    int order = 2;
    std::cout << "BesselJ(" << order << ", " << x << ") = "
              << atom::extra::boost::SpecialFunctions<double>::besselJ(order, x) << std::endl;

    return 0;
}
```

## Statistics

The `Statistics` class provides static methods for computing various statistical measures.

### Methods

```cpp
template <Numeric T>
class Statistics {
public:
    static auto mean(const std::vector<T>& data) -> T;
    static auto variance(const std::vector<T>& data) -> T;
    static auto skewness(const std::vector<T>& data) -> T;
    static auto kurtosis(const std::vector<T>& data) -> T;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};

    std::cout << "Mean: " << atom::extra::boost::Statistics<double>::mean(data) << std::endl;
    std::cout << "Variance: " << atom::extra::boost::Statistics<double>::variance(data) << std::endl;
    std::cout << "Skewness: " << atom::extra::boost::Statistics<double>::skewness(data) << std::endl;
    std::cout << "Kurtosis: " << atom::extra::boost::Statistics<double>::kurtosis(data) << std::endl;

    return 0;
}
```

## Distributions

The `Distributions` class provides nested classes for various probability distributions.

### Nested Classes

- `NormalDistribution`
- `StudentTDistribution`
- `PoissonDistribution`
- `ExponentialDistribution`

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>

int main() {
    atom::extra::boost::Distributions<double>::NormalDistribution normal(0.0, 1.0);
    std::cout << "Normal PDF at x=1: " << normal.pdf(1.0) << std::endl;
    std::cout << "Normal CDF at x=1: " << normal.cdf(1.0) << std::endl;

    atom::extra::boost::Distributions<double>::StudentTDistribution t(10);
    std::cout << "Student's t PDF at x=1: " << t.pdf(1.0) << std::endl;
    std::cout << "Student's t CDF at x=1: " << t.cdf(1.0) << std::endl;

    return 0;
}
```

## NumericalIntegration

The `NumericalIntegration` class provides methods for numerical integration.

### Methods

```cpp
template <Numeric T>
class NumericalIntegration {
public:
    static auto trapezoidal(std::function<T(T)> func, T start, T end) -> T;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>
#include <cmath>

int main() {
    auto func = [](double x) { return std::sin(x); };
    double result = atom::extra::boost::NumericalIntegration<double>::trapezoidal(func, 0.0, M_PI);
    std::cout << "Integral of sin(x) from 0 to pi: " << result << std::endl;

    return 0;
}
```

## Optimization

The `Optimization` class provides methods for optimization problems.

### Methods

```cpp
template <Numeric T>
class Optimization {
public:
    static auto goldenSectionSearch(std::function<T(T)> func, T start, T end, T tolerance) -> T;
    static auto newtonRaphson(std::function<T(T)> func, std::function<T(T)> derivativeFunc,
                              T initialGuess, T tolerance, int maxIterations) -> T;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>
#include <cmath>

int main() {
    auto func = [](double x) { return std::pow(x - 2, 2); };
    auto derivative = [](double x) { return 2 * (x - 2); };

    double minimum = atom::extra::boost::Optimization<double>::goldenSectionSearch(func, 0.0, 4.0, 1e-6);
    std::cout << "Minimum found at x = " << minimum << std::endl;

    double root = atom::extra::boost::Optimization<double>::newtonRaphson(func, derivative, 0.0, 1e-6, 100);
    std::cout << "Root found at x = " << root << std::endl;

    return 0;
}
```

## LinearAlgebra

The `LinearAlgebra` class provides methods for linear algebra operations.

### Methods

```cpp
template <Numeric T>
class LinearAlgebra {
public:
    using Matrix = ::boost::numeric::ublas::matrix<T>;
    using Vector = ::boost::numeric::ublas::vector<T>;

    static auto solveLinearSystem(const Matrix& matrix, const Vector& vector) -> Vector;
    static auto determinant(const Matrix& matrix) -> T;
    static auto multiply(const Matrix& matrix1, const Matrix& matrix2) -> Matrix;
    static auto transpose(const Matrix& matrix) -> Matrix;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>

int main() {
    using Matrix = atom::extra::boost::LinearAlgebra<double>::Matrix;
    using Vector = atom::extra::boost::LinearAlgebra<double>::Vector;

    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 3; A(1, 1) = 4;

    Vector b(2);
    b(0) = 5; b(1) = 11;

    Vector x = atom::extra::boost::LinearAlgebra<double>::solveLinearSystem(A, b);
    std::cout << "Solution: x = " << x(0) << ", y = " << x(1) << std::endl;

    double det = atom::extra::boost::LinearAlgebra<double>::determinant(A);
    std::cout << "Determinant: " << det << std::endl;

    Matrix AT = atom::extra::boost::LinearAlgebra<double>::transpose(A);
    std::cout << "Transpose:" << std::endl;
    for (unsigned i = 0; i < AT.size1(); ++i) {
        for (unsigned j = 0; j < AT.size2(); ++j) {
            std::cout << AT(i, j) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
```

## ODESolver

The `ODESolver` class provides methods for solving ordinary differential equations.

### Methods

```cpp
template <Numeric T>
class ODESolver {
public:
    using State = std::vector<T>;
    using SystemFunction = std::function<void(const State&, State&, T)>;

    static auto rungeKutta4(SystemFunction system, State initialState,
                            T startTime, T endTime, T stepSize) -> std::vector<State>;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>
#include <vector>

int main() {
    // Solve the system of ODEs:
    // dx/dt = y
    // dy/dt = -x
    auto system = [](const std::vector<double>& x, std::vector<double>& dxdt, double /*t*/) {
        dxdt[0] = x[1];
        dxdt[1] = -x[0];
    };

    std::vector<double> initialState = {1.0, 0.0};
    double startTime = 0.0;
    double endTime = 10.0;
    double stepSize = 0.1;

    auto solution = atom::extra::boost::ODESolver<double>::rungeKutta4(system, initialState, startTime, endTime, stepSize);

    std::cout << "Solution at t = " << endTime << ": x = " << solution.back()[0] << ", y = " << solution.back()[1] << std::endl;

    return 0;
}
```

## FinancialMath

The `FinancialMath` class provides methods for financial mathematics calculations.

### Methods

```cpp
template <Numeric T>
class FinancialMath {
public:
    static auto blackScholesCall(T stockPrice, T strikePrice, T riskFreeRate,
                                 T volatility, T timeToMaturity) -> T;
    static auto modifiedDuration(T yield, T couponRate, T faceValue, int periods) -> T;
    static auto bondPrice(T yield, T couponRate, T faceValue, int periods) -> T;
    static auto impliedVolatility(T marketPrice, T stockPrice, T strikePrice,
                                  T riskFreeRate, T timeToMaturity) -> T;
};
```

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>

int main() {
    double S = 100.0;  // Stock price
    double K = 100.0;  // Strike price
    double r = 0.05;   // Risk-free rate
    double sigma = 0.2; // Volatility
    double T = 1.0;    // Time to maturity

    double callPrice = atom::extra::boost::FinancialMath<double>::blackScholesCall(S, K, r, sigma, T);
    std::cout << "Call option price: " << callPrice << std::endl;

    double yield = 0.05;
    double couponRate = 0.06;
    double faceValue = 1000.0;
    int periods = 10;

    double duration = atom::extra::boost::FinancialMath<double>::modifiedDuration(yield, couponRate, faceValue, periods);
    std::cout << "Modified duration: " << duration << std::endl;

    double bondPrice = atom::extra::boost::FinancialMath<double>::bondPrice(yield, couponRate, faceValue, periods);
    std::cout << "Bond price: " << bondPrice << std::endl;

    double marketPrice = 10.0;
    double impliedVol = atom::extra::boost::FinancialMath<double>::impliedVolatility(marketPrice, S, K, r, T);
    std::cout << "Implied volatility: " << impliedVol << std::endl;

    return 0;
}
```

## Utility Functions

The namespace also includes some utility functions for general use.

### factorial

```cpp
template <Numeric T>
constexpr auto factorial(T number) -> T;
```

This function calculates the factorial of a given number. It uses compile-time optimization for integral types and the gamma function for floating-point types.

### transformRange

```cpp
template <std::ranges::input_range Range, typename Func>
auto transformRange(Range&& range, Func func);
```

This function applies a transformation to a range of values using C++20 ranges.

### Usage Example

```cpp
#include "boost_math.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Factorial of 5: " << atom::extra::boost::factorial(5) << std::endl;

    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto squares = atom::extra::boost::transformRange(numbers, [](int x) { return x * x; });

    std::cout << "Squares: ";
    for (auto square : squares) {
        std::cout << square << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

## Conclusion

The `atom::extra::boost` namespace provides a comprehensive set of mathematical and statistical tools built on top of the Boost C++ libraries. It offers a high-level interface for complex mathematical operations, making it easier to perform advanced calculations in C++ applications.

When using this namespace, make sure to link against the necessary Boost libraries and include the appropriate headers. The classes and functions in this namespace are designed to work with any numeric type that satisfies the `Numeric` concept, allowing for flexibility in precision and performance.

For more detailed information on the underlying Boost functions and their behavior, refer to the official Boost documentation.
