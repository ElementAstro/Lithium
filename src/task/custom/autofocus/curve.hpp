#pragma once

#include <vector>
#include <utility>

class FocusCurveFitter {
public:
    FocusCurveFitter(int degree = 4);
    std::pair<int, double> fitCurve(const std::vector<int>& positions, const std::vector<double>& hfrScores);

private:
    int polynomialDegree;
};
