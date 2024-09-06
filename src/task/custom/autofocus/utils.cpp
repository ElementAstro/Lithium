#include "utils.hpp"
#include <numeric>
#include <cmath>

namespace Utils {

std::vector<double> applyNoiseReduction(const std::vector<double>& data) {
    std::vector<double> smoothed = data;
    for (size_t i = 1; i < smoothed.size() - 1; ++i) {
        smoothed[i] = (data[i-1] + data[i] + data[i+1]) / 3.0;
    }
    return smoothed;
}

bool isOutlier(double value, const std::vector<double>& data) {
    if (data.size() < 2) return false;
    
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double sq_sum = std::inner_product(data.begin(), data.end(), data.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / data.size() - mean * mean);
    
    return std::abs(value - mean) > 3 * stdev;
}

} // namespace Utils
