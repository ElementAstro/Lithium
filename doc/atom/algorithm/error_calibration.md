# Advanced Error Calibration Library

## Overview

The Advanced Error Calibration Library is part of the `atom::algorithm` namespace and provides a comprehensive set of tools for error calibration, analysis, and validation. This library is designed to work with floating-point data types and includes methods for linear and polynomial calibration, residual analysis, outlier detection, and cross-validation.

## Table of Contents

1. [Class Definition](#class-definition)
2. [Constructor](#constructor)
3. [Public Methods](#public-methods)
4. [Private Methods](#private-methods)
5. [Usage Examples](#usage-examples)

## Class Definition

```cpp
template <std::floating_point T>
class AdvancedErrorCalibration {
    // ... (class members and methods)
};
```

The `AdvancedErrorCalibration` class is templated to work with any floating-point type.

## Constructor

The class does not have an explicit constructor. Members are initialized with default values.

## Public Methods

### Linear Calibration

```cpp
void linearCalibrate(const std::vector<T>& measured, const std::vector<T>& actual);
```

Performs linear calibration using the least squares method.

- **Parameters:**
  - `measured`: Vector of measured values
  - `actual`: Vector of actual values

### Polynomial Calibration

```cpp
void polynomialCalibrate(const std::vector<T>& measured, const std::vector<T>& actual, int degree);
```

Performs polynomial calibration using the least squares method.

- **Parameters:**
  - `measured`: Vector of measured values
  - `actual`: Vector of actual values
  - `degree`: Degree of the polynomial

### Apply Calibration

```cpp
[[nodiscard]] auto apply(T value) const -> T;
```

Applies the calibration to a single value.

- **Parameters:**
  - `value`: The value to calibrate
- **Returns:** The calibrated value

### Print Parameters

```cpp
void printParameters() const;
```

Prints the calibration parameters (slope, intercept, R-squared, MSE, MAE).

### Get Residuals

```cpp
[[nodiscard]] auto getResiduals() const -> std::vector<T>;
```

Returns the vector of residuals from the last calibration.

### Plot Residuals

```cpp
void plotResiduals(const std::string& filename) const;
```

Writes residual data to a CSV file for plotting.

- **Parameters:**
  - `filename`: The name of the file to write the data to

### Bootstrap Confidence Interval

```cpp
auto bootstrapConfidenceInterval(const std::vector<T>& measured, const std::vector<T>& actual,
                                 int n_iterations = 1000, double confidence_level = 0.95) -> std::pair<T, T>;
```

Calculates the bootstrap confidence interval for the slope.

- **Parameters:**
  - `measured`: Vector of measured values
  - `actual`: Vector of actual values
  - `n_iterations`: Number of bootstrap iterations
  - `confidence_level`: Confidence level for the interval
- **Returns:** A pair of lower and upper bounds of the confidence interval

### Outlier Detection

```cpp
auto outlierDetection(const std::vector<T>& measured, const std::vector<T>& actual,
                      T threshold = 2.0) -> std::tuple<T, T, T>;
```

Detects outliers using the residuals of the calibration.

- **Parameters:**
  - `measured`: Vector of measured values
  - `actual`: Vector of actual values
  - `threshold`: Threshold for outlier detection
- **Returns:** A tuple of mean residual, standard deviation, and threshold

### Cross Validation

```cpp
void crossValidation(const std::vector<T>& measured, const std::vector<T>& actual, int k = 5);
```

Performs k-fold cross-validation.

- **Parameters:**
  - `measured`: Vector of measured values
  - `actual`: Vector of actual values
  - `k`: Number of folds

### Getter Methods

```cpp
[[nodiscard]] auto getSlope() const -> T;
[[nodiscard]] auto getIntercept() const -> T;
[[nodiscard]] auto getRSquared() const -> std::optional<T>;
[[nodiscard]] auto getMse() const -> T;
[[nodiscard]] auto getMae() const -> T;
```

These methods return the calibration parameters and metrics.

## Private Methods

### Calculate Metrics

```cpp
void calculateMetrics(const std::vector<T>& measured, const std::vector<T>& actual);
```

Calculates various metrics after calibration (R-squared, MSE, MAE, residuals).

### Levenberg-Marquardt Algorithm

```cpp
auto levenbergMarquardt(const std::vector<T>& x, const std::vector<T>& y,
                        NonlinearFunction func, std::vector<T> initial_params,
                        int max_iterations = 100, T lambda = 0.01, T epsilon = 1e-8) -> std::vector<T>;
```

Implements the Levenberg-Marquardt algorithm for nonlinear least squares fitting.

### Solve Linear System

```cpp
auto solveLinearSystem(const std::vector<std::vector<T>>& A, const std::vector<T>& b) -> std::vector<T>;
```

Solves a system of linear equations using Gaussian elimination.

## Usage Examples

### Linear Calibration

```cpp
#include "atom/algorithm/error_calibration.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<double> measured = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> actual = {1.1, 2.2, 2.9, 4.1, 5.0};

    atom::algorithm::AdvancedErrorCalibration<double> calibrator;
    calibrator.linearCalibrate(measured, actual);

    calibrator.printParameters();

    double test_value = 3.5;
    std::cout << "Calibrated value of " << test_value << " is "
              << calibrator.apply(test_value) << std::endl;

    return 0;
}
```

### Polynomial Calibration and Outlier Detection

```cpp
#include "atom/algorithm/error_calibration.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<double> measured = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<double> actual = {1.1, 2.2, 2.9, 4.1, 5.0, 7.5};

    atom::algorithm::AdvancedErrorCalibration<double> calibrator;
    calibrator.polynomialCalibrate(measured, actual, 2);  // 2nd degree polynomial

    calibrator.printParameters();

    auto [mean_residual, std_dev, threshold] = calibrator.outlierDetection(measured, actual);
    std::cout << "Outlier detection results:" << std::endl;
    std::cout << "Mean residual: " << mean_residual << std::endl;
    std::cout << "Standard deviation: " << std_dev << std::endl;
    std::cout << "Threshold: " << threshold << std::endl;

    return 0;
}
```

These examples demonstrate how to use the Advanced Error Calibration library for basic linear and polynomial calibration, as well as outlier detection. The library provides a comprehensive set of tools for error analysis and calibration, making it suitable for various scientific and engineering applications.
