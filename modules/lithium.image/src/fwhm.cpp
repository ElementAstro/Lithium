#include "fwhm.hpp"
#include "atom/log/loguru.hpp"

// 常量定义
constexpr int PLOT_HEIGHT = 400;
constexpr int PLOT_WIDTH = 600;
constexpr int WHITE_COLOR = 255;
constexpr int RED_COLOR = 255;
constexpr double WIDTH_FACTOR = 10.0;
constexpr int CIRCLE_RADIUS = 3;
constexpr int CURVE_THICKNESS = 2;

auto GaussianFit::fit(const std::vector<DataPoint>& points, double epsilon,
                      int maxIterations) -> std::optional<GaussianParams> {
    LOG_F(INFO, "Starting Gaussian fit.");
    if (points.empty()) {
        LOG_F(ERROR, "No data points provided.");
        return std::nullopt;
    }

    std::vector<cv::Point2d> fitPoints;
    fitPoints.reserve(points.size());
    for (const auto& point : points) {
        fitPoints.emplace_back(point.x, point.y);
    }

    auto [minY, maxY] = getMinMax(points);
    double baseLevel = minY;
    double peakLevel = maxY - baseLevel;
    double centerPoint = getMeanX(points);
    double widthValue = getEstimatedWidth(points);

    cv::Mat params = (cv::Mat_<double>(4, 1) << baseLevel, peakLevel,
                      centerPoint, widthValue);

    try {
        cv::Mat residuals;
        double prevError = std::numeric_limits<double>::max();

        for (int iteration = 0; iteration < maxIterations; ++iteration) {
            computeResiduals(params, points, residuals);
            LOG_F(INFO, "Iteration %d, error: %f", iteration,
                  cv::norm(residuals));

            if (cv::norm(residuals) >= prevError) {
                LOG_F(WARNING, "Error did not decrease. Stopping iteration.");
                break;
            }

            cv::Mat jacobian;
            computeJacobian(params, points, jacobian);

            cv::Mat delta;
            if (!cv::solve(jacobian.t() * jacobian, -jacobian.t() * residuals,
                           delta, cv::DECOMP_SVD)) {
                LOG_F(ERROR, "Failed to solve for delta.");
                return std::nullopt;
            }

            params += delta;

            double error = cv::norm(residuals);
            if (std::abs(error - prevError) < epsilon) {
                LOG_F(INFO, "Convergence reached.");
                break;
            }
            prevError = error;
        }

        GaussianParams result{params.at<double>(0), params.at<double>(1),
                              params.at<double>(2), params.at<double>(3)};
        LOG_F(INFO, "Gaussian fit successful.");
        return result;
    } catch (const cv::Exception& exception) {
        LOG_F(ERROR, "OpenCV exception: {}", exception.what());
        return std::nullopt;
    }
}

double GaussianFit::evaluate(const GaussianParams& params, double x) {
    double t = (x - params.center) / params.width;
    return params.base + params.peak * std::exp(-0.5 * t * t);
}

void GaussianFit::visualize(const std::vector<DataPoint>& points,
                            const GaussianParams& params) {
    LOG_F(INFO, "Visualizing Gaussian fit.");
    cv::Mat plot(400, 600, CV_8UC3, cv::Scalar(255, 255, 255));

    auto [minIt, maxIt] = std::minmax_element(
        points.begin(), points.end(),
        [](const DataPoint& a, const DataPoint& b) { return a.x < b.x; });
    double xMin = minIt->x, xMax = maxIt->x;

    std::vector<cv::Point> curve;
    for (int i = 0; i < plot.cols; ++i) {
        double x = xMin + (xMax - xMin) * i / plot.cols;
        double y = evaluate(params, x);
        int py = plot.rows - static_cast<int>((y - params.base) * plot.rows /
                                              (params.peak + params.base));
        curve.emplace_back(i, py);
    }
    cv::polylines(plot, curve, false, cv::Scalar(0, 0, 255), 2);

    for (const auto& p : points) {
        int px = static_cast<int>((p.x - xMin) * plot.cols / (xMax - xMin));
        int py = plot.rows - static_cast<int>((p.y - params.base) * plot.rows /
                                              (params.peak + params.base));
        cv::circle(plot, cv::Point(px, py), 3, cv::Scalar(0, 0, 0), -1);
    }

    cv::imshow("Gaussian Fit", plot);
    cv::waitKey(0);
}

void GaussianFit::computeResiduals(const cv::Mat& params,
                                   const std::vector<DataPoint>& points,
                                   cv::Mat& err) {
    err.create(points.size(), 1, CV_64F);
    GaussianParams p{params.at<double>(0), params.at<double>(1),
                     params.at<double>(2), params.at<double>(3)};

    for (size_t i = 0; i < points.size(); i++) {
        err.at<double>(i) = points[i].y - evaluate(p, points[i].x);
    }
}

void GaussianFit::computeJacobian(const cv::Mat& params,
                                  const std::vector<DataPoint>& points,
                                  cv::Mat& jac) {
    jac.create(points.size(), 4, CV_64F);
    double base = params.at<double>(0);
    double peak = params.at<double>(1);
    double center = params.at<double>(2);
    double width = params.at<double>(3);

    for (size_t i = 0; i < points.size(); i++) {
        double x = points[i].x;
        double t = (x - center) / width;
        double expTerm = std::exp(-0.5 * t * t);

        jac.at<double>(i, 0) = -1.0;
        jac.at<double>(i, 1) = -expTerm;
        jac.at<double>(i, 2) = -peak * expTerm * t / width;
        jac.at<double>(i, 3) = -peak * expTerm * t * t / width;
    }
}

std::pair<double, double> GaussianFit::getMinMax(
    const std::vector<DataPoint>& points) {
    auto [min, max] = std::minmax_element(
        points.begin(), points.end(),
        [](const DataPoint& a, const DataPoint& b) { return a.y < b.y; });
    return {min->y, max->y};
}

double GaussianFit::getMeanX(const std::vector<DataPoint>& points) {
    double sum = 0;
    for (const auto& p : points) {
        sum += p.x;
    }
    return sum / points.size();
}

double GaussianFit::getEstimatedWidth(const std::vector<DataPoint>& points) {
    auto [minX, maxX] = std::minmax_element(
        points.begin(), points.end(),
        [](const DataPoint& a, const DataPoint& b) { return a.x < b.x; });
    return (maxX->x - minX->x) / 10.0;
}