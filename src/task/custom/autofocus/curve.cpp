#include "curve.hpp"
#include <Eigen/Dense>
#include <cmath>

FocusCurveFitter::FocusCurveFitter(int degree) : polynomialDegree(degree) {}

std::pair<int, double> FocusCurveFitter::fitCurve(const std::vector<int>& positions, const std::vector<double>& hfrScores) {
    int n = positions.size();
    Eigen::MatrixXd A(n, polynomialDegree + 1);
    Eigen::VectorXd b(n);

    for (int i = 0; i < n; ++i) {
        double x = positions[i];
        for (int j = 0; j <= polynomialDegree; ++j) {
            A(i, j) = std::pow(x, j);
        }
        b(i) = hfrScores[i];
    }

    Eigen::VectorXd coeffs = A.colPivHouseholderQr().solve(b);

    std::vector<double> roots;
    for (int i = 1; i < polynomialDegree; ++i) {
        double discriminant = coeffs[i+1]*coeffs[i+1] - 4*coeffs[i+2]*coeffs[i];
        if (discriminant >= 0) {
            roots.push_back((-coeffs[i+1] + std::sqrt(discriminant)) / (2*coeffs[i+2]));
            roots.push_back((-coeffs[i+1] - std::sqrt(discriminant)) / (2*coeffs[i+2]));
        }
    }

    int bestFitPosition = 0;
    double bestFitHFR = std::numeric_limits<double>::max();
    for (double root : roots) {
        if (root >= positions.front() && root <= positions.back()) {
            double hfr = 0;
            for (int i = 0; i <= polynomialDegree; ++i) {
                hfr += coeffs[i] * std::pow(root, i);
            }
            if (hfr < bestFitHFR) {
                bestFitHFR = hfr;
                bestFitPosition = static_cast<int>(std::round(root));
            }
        }
    }

    return {bestFitPosition, bestFitHFR};
}
