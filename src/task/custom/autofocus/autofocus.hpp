#pragma once

#include <deque>
#include <opencv2/opencv.hpp>
#include <vector>

class StarDetector;
class FocusCurveFitter;

class AstroAutoFocus {
public:
    AstroAutoFocus();
    void focus(const std::vector<cv::Mat>& images,
               const std::vector<int>& positions, double temperature);
    int getFocusPosition() const;
    double getBestHFR() const;
    std::vector<std::pair<int, double>> getFocusCurve() const;
    void setTemperature(double temperature);

private:
    static const int MAX_FOCUS_STEPS = 100;
    static const double HFR_THRESHOLD;
    static const int WINDOW_SIZE = 5;
    static const double TEMPERATURE_COEFFICIENT;
    static const int HISTORY_SIZE = 10;

    std::vector<double> hfrScores;
    std::vector<int> focusPositions;
    int currentPosition;
    int bestPosition;
    double bestHFR;
    double currentTemperature;
    std::deque<std::pair<double, int>> focusHistory;

    StarDetector* starDetector;
    FocusCurveFitter* curveFitter;

    double calculateHFR(const cv::Mat& image);
    bool isPeak(int index);
    bool isFalsePeak(int index);
    int calculateAdaptiveStep(int currentStep, double previousHFR,
                              double currentHFR);
    void updateFocusHistory(int position, double hfr);
    int getTemperatureCompensatedPosition(int position);
};
