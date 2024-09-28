#ifndef ATOM_EXTRA_BOOST_MATH_HPP
#define ATOM_EXTRA_BOOST_MATH_HPP

#include <boost/math/distributions.hpp>
#include <boost/math/quadrature/trapezoidal.hpp>
#include <boost/math/special_functions.hpp>
#include <boost/math/statistics/univariate_statistics.hpp>

#include <boost/numeric/odeint.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <functional>
#include <ranges>
#include <type_traits>
#include <vector>

namespace atom::extra::boost {

// 概念定义
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

// 特殊函数封装
template <Numeric T>
class SpecialFunctions {
public:
    static auto beta(T alpha, T beta) -> T {
        return ::boost::math::beta(alpha, beta);
    }
    static auto gamma(T value) -> T { return ::boost::math::tgamma(value); }
    static auto digamma(T value) -> T { return ::boost::math::digamma(value); }
    static auto erf(T value) -> T { return ::boost::math::erf(value); }
    static auto besselJ(int order, T value) -> T {
        return ::boost::math::cyl_bessel_j(order, value);
    }
    static auto legendreP(int order, T value) -> T {
        return ::boost::math::legendre_p(order, value);
    }
};

// 统计函数封装
template <Numeric T>
class Statistics {
public:
    static auto mean(const std::vector<T>& data) -> T {
        return ::boost::math::statistics::mean(data);
    }

    static auto variance(const std::vector<T>& data) -> T {
        return ::boost::math::statistics::variance(data);
    }

    static auto skewness(const std::vector<T>& data) -> T {
        return ::boost::math::statistics::skewness(data);
    }

    static auto kurtosis(const std::vector<T>& data) -> T {
        return ::boost::math::statistics::kurtosis(data);
    }
};

// 概率分布封装
template <Numeric T>
class Distributions {
public:
    class NormalDistribution {
    private:
        ::boost::math::normal_distribution<T> distribution;

    public:
        NormalDistribution(T mean, T stddev) : distribution(mean, stddev) {}

        [[nodiscard]] auto pdf(T value) const -> T {
            return ::boost::math::pdf(distribution, value);
        }
        [[nodiscard]] auto cdf(T value) const -> T {
            return ::boost::math::cdf(distribution, value);
        }
        [[nodiscard]] auto quantile(T probability) const -> T {
            return ::boost::math::quantile(distribution, probability);
        }
    };

    class StudentTDistribution {
    private:
        ::boost::math::students_t_distribution<T> distribution;

    public:
        explicit StudentTDistribution(T degreesOfFreedom)
            : distribution(degreesOfFreedom) {}

        [[nodiscard]] auto pdf(T value) const -> T {
            return ::boost::math::pdf(distribution, value);
        }
        [[nodiscard]] auto cdf(T value) const -> T {
            return ::boost::math::cdf(distribution, value);
        }
        [[nodiscard]] auto quantile(T probability) const -> T {
            return ::boost::math::quantile(distribution, probability);
        }
    };

    class PoissonDistribution {
    private:
        ::boost::math::poisson_distribution<T> distribution;

    public:
        explicit PoissonDistribution(T mean) : distribution(mean) {}

        [[nodiscard]] auto pdf(T value) const -> T {
            return ::boost::math::pdf(distribution, value);
        }
        [[nodiscard]] auto cdf(T value) const -> T {
            return ::boost::math::cdf(distribution, value);
        }
    };

    class ExponentialDistribution {
    private:
        ::boost::math::exponential_distribution<T> distribution;

    public:
        explicit ExponentialDistribution(T lambda) : distribution(lambda) {}

        [[nodiscard]] auto pdf(T value) const -> T {
            return ::boost::math::pdf(distribution, value);
        }
        [[nodiscard]] auto cdf(T value) const -> T {
            return ::boost::math::cdf(distribution, value);
        }
    };
};

// 数值积分封装
template <Numeric T>
class NumericalIntegration {
public:
    static auto trapezoidal(std::function<T(T)> func, T start, T end) -> T {
        return ::boost::math::quadrature::trapezoidal(func, start, end);
    }
};

// 使用C++20的constexpr if来优化编译时计算
template <Numeric T>
constexpr auto factorial(T number) -> T {
    if constexpr (std::is_integral_v<T>) {
        if (number == 0 || number == 1) {
            return 1;
        }
        return number * factorial(number - 1);
    } else {
        return std::tgamma(number + 1);
    }
}

// 使用C++20的ranges来处理数据序列
template <std::ranges::input_range Range, typename Func>
auto transformRange(Range&& range, Func func) {
    return std::ranges::transform_view(std::forward<Range>(range), func);
}

template <Numeric T>
class Optimization {
public:
    // 一维黄金分割搜索
    static auto goldenSectionSearch(std::function<T(T)> func, T start, T end,
                                    T tolerance) -> T {
        const T goldenRatio = 0.618033988749895;
        T pointC = end - goldenRatio * (end - start);
        T pointD = start + goldenRatio * (end - start);

        while (std::abs(pointC - pointD) > tolerance) {
            if (func(pointC) < func(pointD)) {
                end = pointD;
            } else {
                start = pointC;
            }
            pointC = end - goldenRatio * (end - start);
            pointD = start + goldenRatio * (end - start);
        }

        return (start + end) / 2;
    }

    // 牛顿法求根
    static auto newtonRaphson(std::function<T(T)> func,
                              std::function<T(T)> derivativeFunc,
                              T initialGuess, T tolerance,
                              int maxIterations) -> T {
        T currentGuess = initialGuess;
        for (int i = 0; i < maxIterations; ++i) {
            T funcValue = func(currentGuess);
            if (std::abs(funcValue) < tolerance) {
                return currentGuess;
            }
            T derivativeValue = derivativeFunc(currentGuess);
            if (derivativeValue == 0) {
                throw std::runtime_error(
                    "Derivative is zero. Cannot continue.");
            }
            currentGuess = currentGuess - funcValue / derivativeValue;
        }
        throw std::runtime_error("Max iterations reached without convergence.");
    }
};

// 线性代数操作
template <Numeric T>
class LinearAlgebra {
public:
    using Matrix = ::boost::numeric::ublas::matrix<T>;
    using Vector = ::boost::numeric::ublas::vector<T>;

    static auto solveLinearSystem(const Matrix& matrix,
                                  const Vector& vector) -> Vector {
        // 使用LU分解求解线性系统 Ax = b
        ::boost::numeric::ublas::permutation_matrix<std::size_t>
            permutationMatrix(matrix.size1());
        Matrix matrixCopy = matrix;
        ::boost::numeric::ublas::lu_factorize(matrixCopy, permutationMatrix);
        Vector solution = vector;
        ::boost::numeric::ublas::lu_substitute(matrixCopy, permutationMatrix,
                                               solution);
        return solution;
    }

    static auto determinant(const Matrix& matrix) -> T {
        // 计算矩阵行列式
        Matrix matrixCopy = matrix;
        ::boost::numeric::ublas::permutation_matrix<std::size_t>
            permutationMatrix(matrix.size1());
        ::boost::numeric::ublas::lu_factorize(matrixCopy, permutationMatrix);
        T determinantValue = 1.0;
        for (std::size_t i = 0; i < matrix.size1(); ++i) {
            determinantValue *= matrixCopy(i, i);
        }
        return determinantValue * (permutationMatrix.size() % 2 == 1 ? -1 : 1);
    }

    static auto multiply(const Matrix& matrix1,
                         const Matrix& matrix2) -> Matrix {
        return ::boost::numeric::ublas::prod(matrix1, matrix2);
    }

    static auto transpose(const Matrix& matrix) -> Matrix {
        return ::boost::numeric::ublas::trans(matrix);
    }
};

// 微分方程求解器
template <Numeric T>
class ODESolver {
public:
    using State = std::vector<T>;
    using SystemFunction = std::function<void(const State&, State&, T)>;

    static auto rungeKutta4(SystemFunction system, State initialState,
                            T startTime, T endTime,
                            T stepSize) -> std::vector<State> {
        std::vector<State> solution;
        ::boost::numeric::odeint::runge_kutta4<State> stepper;
        ::boost::numeric::odeint::integrate_const(
            stepper, system, initialState, startTime, endTime, stepSize,
            [&solution](const State& state, T) { solution.push_back(state); });
        return solution;
    }
};

// 金融数学函数
template <Numeric T>
class FinancialMath {
public:
    // Black-Scholes公式计算欧式看涨期权价格
    static auto blackScholesCall(T stockPrice, T strikePrice, T riskFreeRate,
                                 T volatility, T timeToMaturity) -> T {
        T d1 =
            (std::log(stockPrice / strikePrice) +
             (riskFreeRate + 0.5 * volatility * volatility) * timeToMaturity) /
            (volatility * std::sqrt(timeToMaturity));
        T d2 = d1 - volatility * std::sqrt(timeToMaturity);
        return stockPrice * ::boost::math::cdf(
                                ::boost::math::normal_distribution<T>(), d1) -
               strikePrice * std::exp(-riskFreeRate * timeToMaturity) *
                   ::boost::math::cdf(::boost::math::normal_distribution<T>(),
                                      d2);
    }

    // 计算债券的修正久期
    static auto modifiedDuration(T yield, T couponRate, T faceValue,
                                 int periods) -> T {
        T periodYield = yield / periods;
        T couponPayment = couponRate * faceValue / periods;
        T numPeriods = static_cast<T>(periods);
        T presentValue = 0;
        T weightedPresentValue = 0;
        for (int i = 1; i <= periods; ++i) {
            T discountFactor = std::pow(1 + periodYield, -i);
            presentValue += couponPayment * discountFactor;
            weightedPresentValue += i * couponPayment * discountFactor;
        }
        presentValue += faceValue * std::pow(1 + periodYield, -numPeriods);
        weightedPresentValue +=
            numPeriods * faceValue * std::pow(1 + periodYield, -numPeriods);
        return (weightedPresentValue / presentValue) / (1 + periodYield);
    }

    // 计算债券价格
    static auto bondPrice(T yield, T couponRate, T faceValue,
                          int periods) -> T {
        T periodYield = yield / periods;
        T couponPayment = couponRate * faceValue / periods;
        T presentValue = 0;
        for (int i = 1; i <= periods; ++i) {
            presentValue += couponPayment * std::pow(1 + periodYield, -i);
        }
        presentValue += faceValue * std::pow(1 + periodYield, -periods);
        return presentValue;
    }

    // 计算期权的隐含波动率
    static auto impliedVolatility(T marketPrice, T stockPrice, T strikePrice,
                                  T riskFreeRate, T timeToMaturity) -> T {
        auto objectiveFunction = [&](T volatility) {
            return blackScholesCall(stockPrice, strikePrice, riskFreeRate,
                                    volatility, timeToMaturity) -
                   marketPrice;
        };
        return Optimization<T>::newtonRaphson(
            objectiveFunction, [](T) { return 1; }, 0.2, 1e-6, 100);
    }
};

}  // namespace atom::extra::boost

#endif
