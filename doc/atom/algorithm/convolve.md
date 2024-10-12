# Atom Algorithm Library: Convolution and Deconvolution

This document provides a detailed explanation of the convolution and deconvolution functions implemented in the Atom Algorithm Library. These functions are part of the `atom::algorithm` namespace and are designed for signal and image processing tasks.

## Table of Contents

1. [Overview](#overview)
2. [1D Convolution and Deconvolution](#1d-convolution-and-deconvolution)
3. [2D Convolution and Deconvolution](#2d-convolution-and-deconvolution)
4. [Fourier Transform Functions](#fourier-transform-functions)
5. [Gaussian Filter Functions](#gaussian-filter-functions)
6. [Usage Examples](#usage-examples)

## Overview

The library provides functions for both one-dimensional and two-dimensional convolution and deconvolution operations. It also includes functions for Discrete Fourier Transform (DFT) and Inverse Discrete Fourier Transform (IDFT), as well as utilities for generating and applying Gaussian filters.

## 1D Convolution and Deconvolution

### 1D Convolution

```cpp
[[nodiscard]] auto convolve(const std::vector<double>& input,
                            const std::vector<double>& kernel) -> std::vector<double>;
```

Performs one-dimensional convolution of the input signal with the given kernel.

- **Parameters:**
  - `input`: The input signal as a vector of doubles.
  - `kernel`: The convolution kernel as a vector of doubles.
- **Returns:** A vector of doubles representing the convolved signal.

### 1D Deconvolution

```cpp
[[nodiscard]] auto deconvolve(const std::vector<double>& input,
                              const std::vector<double>& kernel) -> std::vector<double>;
```

Performs one-dimensional deconvolution of the input signal with the given kernel.

- **Parameters:**
  - `input`: The input signal as a vector of doubles.
  - `kernel`: The deconvolution kernel as a vector of doubles.
- **Returns:** A vector of doubles representing the deconvolved signal.

## 2D Convolution and Deconvolution

### 2D Convolution

```cpp
[[nodiscard]] auto convolve2D(const std::vector<std::vector<double>>& input,
                              const std::vector<std::vector<double>>& kernel,
                              int numThreads = 1) -> std::vector<std::vector<double>>;
```

Performs two-dimensional convolution of the input image with the given kernel.

- **Parameters:**
  - `input`: The input image as a 2D vector of doubles.
  - `kernel`: The convolution kernel as a 2D vector of doubles.
  - `numThreads`: Number of threads for parallel execution (default: 1).
- **Returns:** A 2D vector of doubles representing the convolved image.

### 2D Deconvolution

```cpp
[[nodiscard]] auto deconvolve2D(const std::vector<std::vector<double>>& signal,
                                const std::vector<std::vector<double>>& kernel,
                                int numThreads = 1) -> std::vector<std::vector<double>>;
```

Performs two-dimensional deconvolution of the input image with the given kernel.

- **Parameters:**
  - `signal`: The input image as a 2D vector of doubles.
  - `kernel`: The deconvolution kernel as a 2D vector of doubles.
  - `numThreads`: Number of threads for parallel execution (default: 1).
- **Returns:** A 2D vector of doubles representing the deconvolved image.

## Fourier Transform Functions

### 2D Discrete Fourier Transform (DFT)

```cpp
[[nodiscard]] auto dfT2D(const std::vector<std::vector<double>>& signal,
                         int numThreads = 1) -> std::vector<std::vector<std::complex<double>>>;
```

Computes the two-dimensional Discrete Fourier Transform of the input image.

- **Parameters:**
  - `signal`: The input image as a 2D vector of doubles.
  - `numThreads`: Number of threads for parallel execution (default: 1).
- **Returns:** A 2D vector of complex doubles representing the DFT spectrum.

### 2D Inverse Discrete Fourier Transform (IDFT)

```cpp
[[nodiscard]] auto idfT2D(const std::vector<std::vector<std::complex<double>>>& spectrum,
                          int numThreads = 1) -> std::vector<std::vector<double>>;
```

Computes the two-dimensional Inverse Discrete Fourier Transform of the input spectrum.

- **Parameters:**
  - `spectrum`: The input spectrum as a 2D vector of complex doubles.
  - `numThreads`: Number of threads for parallel execution (default: 1).
- **Returns:** A 2D vector of doubles representing the IDFT image.

## Gaussian Filter Functions

### Generate Gaussian Kernel

```cpp
[[nodiscard]] auto generateGaussianKernel(int size, double sigma) -> std::vector<std::vector<double>>;
```

Generates a Gaussian kernel for 2D convolution.

- **Parameters:**
  - `size`: The size of the kernel (width and height).
  - `sigma`: The standard deviation of the Gaussian distribution.
- **Returns:** A 2D vector of doubles representing the Gaussian kernel.

### Apply Gaussian Filter

```cpp
[[nodiscard]] auto applyGaussianFilter(const std::vector<std::vector<double>>& image,
                                       const std::vector<std::vector<double>>& kernel) -> std::vector<std::vector<double>>;
```

Applies a Gaussian filter to an image.

- **Parameters:**
  - `image`: The input image as a 2D vector of doubles.
  - `kernel`: The Gaussian kernel as a 2D vector of doubles.
- **Returns:** A 2D vector of doubles representing the filtered image.

## Usage Examples

Here are some examples demonstrating how to use the functions in this library:

### 1D Convolution Example

```cpp
#include "atom/algorithm/convolve.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<double> input = {1, 2, 3, 4, 5};
    std::vector<double> kernel = {0.5, 0.5};

    auto result = atom::algorithm::convolve(input, kernel);

    std::cout << "Convolution result:" << std::endl;
    for (const auto& val : result) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### 2D Convolution with Gaussian Filter Example

```cpp
#include "atom/algorithm/convolve.hpp"
#include <iostream>
#include <vector>

int main() {
    // Create a sample 5x5 image
    std::vector<std::vector<double>> image = {
        {1, 2, 3, 4, 5},
        {2, 3, 4, 5, 6},
        {3, 4, 5, 6, 7},
        {4, 5, 6, 7, 8},
        {5, 6, 7, 8, 9}
    };

    // Generate a 3x3 Gaussian kernel
    auto kernel = atom::algorithm::generateGaussianKernel(3, 1.0);

    // Apply the Gaussian filter
    auto filtered_image = atom::algorithm::applyGaussianFilter(image, kernel);

    std::cout << "Filtered image:" << std::endl;
    for (const auto& row : filtered_image) {
        for (const auto& val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
```

These examples demonstrate basic usage of the convolution and Gaussian filtering functions. The library provides powerful tools for signal and image processing tasks, allowing for efficient manipulation of 1D and 2D data.
