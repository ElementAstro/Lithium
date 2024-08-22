#include "atom/algorithm/convolve.hpp"

#include <iostream>

int main() {
    {
        std::vector<double> signal = {1, 2, 3, 4, 5};
        std::vector<double> kernel = {0.2, 0.5, 0.2};

        std::vector<double> result = atom::algorithm::convolve(signal, kernel);

        std::cout << "1D Convolution result: ";
        for (double val : result) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    {
        std::vector<double> signal = {0.2, 0.9, 2.0, 3.1, 2.8, 1.0};
        std::vector<double> kernel = {0.2, 0.5, 0.2};

        std::vector<double> result =
            atom::algorithm::deconvolve(signal, kernel);

        std::cout << "1D Deconvolution result: ";
        for (double val : result) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    {
        std::vector<std::vector<double>> image = {
            {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        std::vector<std::vector<double>> kernel = {
            {1, 0, -1}, {1, 0, -1}, {1, 0, -1}};

        std::vector<std::vector<double>> result =
            atom::algorithm::convolve2D(image, kernel);

        std::cout << "2D Convolution result:" << std::endl;
        for (const auto& row : result) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    {
        std::vector<std::vector<double>> image = {
            {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        std::vector<std::vector<double>> kernel = {
            {1, 0, -1}, {1, 0, -1}, {1, 0, -1}};

        std::vector<std::vector<double>> result =
            atom::algorithm::deconvolve2D(image, kernel);

        std::cout << "2D Deconvolution result:" << std::endl;
        for (const auto& row : result) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    {
        std::vector<std::vector<double>> image = {
            {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        std::vector<std::vector<std::complex<double>>> result =
            atom::algorithm::dfT2D(image);

        std::cout << "2D DFT result:" << std::endl;
        for (const auto& row : result) {
            for (const auto& val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    {
        std::vector<std::vector<std::complex<double>>> spectrum = {
            {std::complex<double>(45, 0),
             std::complex<double>(-4.5, 2.598076211353316)},
            {std::complex<double>(-13.5, 7.794228634059948),
             std::complex<double>(0, 0)},
            {std::complex<double>(-13.5, -7.794228634059948),
             std::complex<double>(-4.5, -2.598076211353316)}};

        std::vector<std::vector<double>> result =
            atom::algorithm::idfT2D(spectrum);

        std::cout << "2D IDFT result:" << std::endl;
        for (const auto& row : result) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    {
        int size = 5;
        double sigma = 1.0;

        std::vector<std::vector<double>> kernel =
            atom::algorithm::generateGaussianKernel(size, sigma);

        std::cout << "Gaussian Kernel:" << std::endl;
        for (const auto& row : kernel) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    {
        std::vector<std::vector<double>> image = {
            {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

        int size = 3;
        double sigma = 1.0;
        std::vector<std::vector<double>> kernel =
            atom::algorithm::generateGaussianKernel(size, sigma);

        std::vector<std::vector<double>> result =
            atom::algorithm::applyGaussianFilter(image, kernel);

        std::cout << "Gaussian Filter result:" << std::endl;
        for (const auto& row : result) {
            for (double val : row) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
