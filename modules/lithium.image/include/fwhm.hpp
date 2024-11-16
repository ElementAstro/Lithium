#pragma once

#include <opencv2/opencv.hpp>

#include <optional>
#include <vector>

struct DataPoint {
    double x;
    double y;
    DataPoint(double x = 0, double y = 0) : x(x), y(y) {}
};

struct GaussianParams {
    double base;
    double peak;
    double center;
    double width;
};

class GaussianFit {
public:
    static std::optional<GaussianParams> fit(
        const std::vector<DataPoint>& points, double eps = 1e-6,
        int maxIter = 100);

    static double evaluate(const GaussianParams& params, double x);

    static void visualize(const std::vector<DataPoint>& points,
                          const GaussianParams& params);

private:
    static void computeResiduals(const cv::Mat& params,
                                 const std::vector<DataPoint>& points,
                                 cv::Mat& err);

    static void computeJacobian(const cv::Mat& params,
                                const std::vector<DataPoint>& points,
                                cv::Mat& jac);

    static std::pair<double, double> getMinMax(
        const std::vector<DataPoint>& points);

    static double getMeanX(const std::vector<DataPoint>& points);

    static double getEstimatedWidth(const std::vector<DataPoint>& points);
};