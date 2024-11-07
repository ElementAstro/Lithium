#ifndef FOCUS_CURVE_FITTER_H
#define FOCUS_CURVE_FITTER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "atom/macro.hpp"

enum class ModelType { POLYNOMIAL, GAUSSIAN, LORENTZIAN };

struct DataPoint {
    double position;
    double sharpness;
} ATOM_ALIGNAS(16);

class FocusCurveFitter {
public:
    FocusCurveFitter();
    ~FocusCurveFitter();

    void addDataPoint(double position, double sharpness);
    auto fitCurve() -> std::vector<double>;
    void autoSelectModel();
    auto calculateConfidenceIntervals(double confidence_level = 0.95)
        -> std::vector<std::pair<double, double>>;
    void visualize(const std::string& filename = "focus_curve.png");
    void preprocessData();
    void realTimeFitAndPredict(double new_position);
    void parallelFitting();
    void saveFittedCurve(const std::string& filename);
    void loadFittedCurve(const std::string& filename);

private:
    class Impl;                   // Forward declaration
    std::unique_ptr<Impl> impl_;  // Smart pointer for Pimpl
};

#endif  // FOCUS_CURVE_FITTER_H