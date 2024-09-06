#pragma once

#include <vector>

namespace Utils {
    std::vector<double> applyNoiseReduction(const std::vector<double>& data);
    bool isOutlier(double value, const std::vector<double>& data);
}
