#include "autofocus.hpp"
#include <algorithm>
#include <numeric>
#include "curve.hpp"
#include "detector.cpp"
#include "utils.hpp"

const double AstroAutoFocus::HFR_THRESHOLD = 0.1;
const double AstroAutoFocus::TEMPERATURE_COEFFICIENT = 0.001;

AstroAutoFocus::AstroAutoFocus()
    : currentPosition(0),
      bestPosition(0),
      bestHFR(std::numeric_limits<double>::max()),
      currentTemperature(20.0),
      starDetector(new StarDetector()),
      curveFitter(new FocusCurveFitter()) {}

void AstroAutoFocus::focus(const std::vector<cv::Mat>& images,
                           const std::vector<int>& positions,
                           double temperature) {
    hfrScores.clear();
    focusPositions = positions;
    bestHFR = std::numeric_limits<double>::max();
    currentTemperature = temperature;

    int step = std::abs(positions[1] - positions[0]);

    for (int i = 0;
         i < std::min(static_cast<int>(images.size()), MAX_FOCUS_STEPS); ++i) {
        double hfr = calculateHFR(images[i]);
        if (!Utils::isOutlier(hfr, hfrScores)) {
            hfrScores.push_back(hfr);

            if (hfr < bestHFR) {
                bestHFR = hfr;
                bestPosition = focusPositions[i];
            }

            if (isPeak(i) && !isFalsePeak(i)) {
                currentPosition = focusPositions[i];
                break;
            }

            if (i > 0) {
                step = calculateAdaptiveStep(step, hfrScores[i - 1], hfr);
            }

            if (i > WINDOW_SIZE &&
                std::all_of(hfrScores.end() - WINDOW_SIZE, hfrScores.end(),
                            [&](double h) { return h > bestHFR; })) {
                break;
            }
        }
    }

    std::vector<double> smoothedHFR = Utils::applyNoiseReduction(hfrScores);

    auto [fitPosition, fitHFR] =
        curveFitter->fitCurve(focusPositions, smoothedHFR);
    if (fitHFR < bestHFR) {
        bestPosition = fitPosition;
        bestHFR = fitHFR;
    }

    currentPosition = getTemperatureCompensatedPosition(bestPosition);
    updateFocusHistory(currentPosition, bestHFR);
}

double AstroAutoFocus::calculateHFR(const cv::Mat& image) {
    std::vector<StarDetector::Star> stars = starDetector->detectStars(image);
    if (stars.empty()) {
        return std::numeric_limits<double>::max();
    }
    double totalHFR = 0;
    for (const auto& star : stars) {
        totalHFR += star.hfr;
    }
    return totalHFR / stars.size();
}

bool AstroAutoFocus::isPeak(int index) {
    if (index <= 0 || index >= hfrScores.size() - 1)
        return false;
    return hfrScores[index] < hfrScores[index - 1] &&
           hfrScores[index] < hfrScores[index + 1];
}

bool AstroAutoFocus::isFalsePeak(int index) {
    if (index <= WINDOW_SIZE / 2 || index >= hfrScores.size() - WINDOW_SIZE / 2)
        return false;

    double localMean = 0;
    for (int i = index - WINDOW_SIZE / 2; i <= index + WINDOW_SIZE / 2; ++i) {
        localMean += hfrScores[i];
    }
    localMean /= WINDOW_SIZE;

    double localStdDev = 0;
    for (int i = index - WINDOW_SIZE / 2; i <= index + WINDOW_SIZE / 2; ++i) {
        localStdDev += std::pow(hfrScores[i] - localMean, 2);
    }
    localStdDev = std::sqrt(localStdDev / WINDOW_SIZE);

    return localStdDev < HFR_THRESHOLD;
}

int AstroAutoFocus::calculateAdaptiveStep(int currentStep, double previousHFR,
                                          double currentHFR) {
    double hfrChange = std::abs(currentHFR - previousHFR);
    if (hfrChange < HFR_THRESHOLD / 2) {
        return currentStep * 2;
    } else if (hfrChange > HFR_THRESHOLD * 2) {
        return std::max(1, currentStep / 2);
    }
    return currentStep;
}

void AstroAutoFocus::updateFocusHistory(int position, double hfr) {
    focusHistory.push_front({currentTemperature, position});
    if (focusHistory.size() > HISTORY_SIZE) {
        focusHistory.pop_back();
    }
}

int AstroAutoFocus::getTemperatureCompensatedPosition(int position) {
    if (focusHistory.empty()) {
        return position;
    }
    double tempDiff = currentTemperature - focusHistory.front().first;
    int compensation = static_cast<int>(tempDiff * TEMPERATURE_COEFFICIENT);
    return position + compensation;
}

int AstroAutoFocus::getFocusPosition() const { return currentPosition; }

double AstroAutoFocus::getBestHFR() const { return bestHFR; }

std::vector<std::pair<int, double>> AstroAutoFocus::getFocusCurve() const {
    std::vector<std::pair<int, double>> curve;
    for (size_t i = 0; i < focusPositions.size(); ++i) {
        curve.emplace_back(focusPositions[i], hfrScores[i]);
    }
    return curve;
}

void AstroAutoFocus::setTemperature(double temperature) {
    currentTemperature = temperature;
}
