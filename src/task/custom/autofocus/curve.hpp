#ifndef FOCUS_CURVE_FITTER_H
#define FOCUS_CURVE_FITTER_H

#include <string>
#include <utility>
#include <vector>

enum class ModelType { POLYNOMIAL, GAUSSIAN, LORENTZIAN };

struct DataPoint {
    double position;
    double sharpness;
};

class FocusCurveFitter {
public:
    FocusCurveFitter();
    ~FocusCurveFitter();

    void addDataPoint(double position, double sharpness);
    std::vector<double> fitCurve();
    void autoSelectModel();
    std::vector<std::pair<double, double>> calculateConfidenceIntervals(
        double confidence_level = 0.95);
    void visualize(const std::string& filename = "focus_curve.png");
    void preprocessData();
    void realTimeFitAndPredict(double new_position);
    void parallelFitting();

private:
    class Impl;   // Forward declaration of the implementation class
    Impl* impl_;  // Pointer to implementation (Pimpl idiom)
};

#endif  // FOCUS_CURVE_FITTER_H
