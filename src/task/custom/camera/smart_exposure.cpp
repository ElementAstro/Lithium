#include "smart_exposure.hpp"
#include "task/utils/macro.hpp"
#include <chrono>
#include <random>
#include <string>
#include <thread>

#include "config/configor.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium {

constexpr double MAX_EXPOSURE_TIME = 3600.0;
constexpr double QUALITY_THRESHOLD_LOW = 0.3;
constexpr double QUALITY_THRESHOLD_HIGH = 0.7;
constexpr double QUALITY_THRESHOLD_VERY_HIGH = 0.9;

class SmartExposure::Impl {
public:
    std::string camera_name;
    double exposure_time;

    // Additional exposure-specific settings
    int gain;
    int max_gain;
    int min_gain;

    std::shared_ptr<ConfigManager> config_manager;
    std::shared_ptr<TaskScheduler> task_scheduler;

    Impl()
        : camera_name(""), exposure_time(0.0), gain(0), max_gain(0), min_gain(0),
          config_manager(nullptr), task_scheduler(nullptr) {}
};

SmartExposure::SmartExposure(const json& params) {
    impl_ = std::make_unique<Impl>();
    impl_->config_manager = std::make_shared<ConfigManager>();
    impl_->task_scheduler = std::make_shared<TaskScheduler>();

    // Extracting parameters
    impl_->camera_name = params.at("camera_name").get<std::string>();
    impl_->exposure_time = params.at("exposure_time").get<double>();
    impl_->gain = params.at("gain").get<int>();
}

auto SmartExposure::validateExposure() -> TaskScheduler::Task {
    if (impl_->exposure_time <= 0 || impl_->exposure_time > MAX_EXPOSURE_TIME) {
        LOG_F(INFO, "Invalid exposure time: {}", impl_->exposure_time);
        throw std::invalid_argument("Exposure time is out of range.");
    }

    int maxGain = impl_->config_manager->getValue("/camera/gain/max")->get<int>();
    int minGain = impl_->config_manager->getValue("/camera/gain/min")->get<int>();
    if (impl_->gain < minGain || impl_->gain > maxGain) {
        LOG_F(INFO, "Invalid gain: {}", impl_->gain);
        throw std::invalid_argument("Gain is out of range.");
    }

    co_return "Validation successful";
}

auto SmartExposure::takeExposure() -> TaskScheduler::Task {
    try {
        LOG_F(INFO, "Starting exposure: {} seconds for camera {}", impl_->exposure_time, impl_->camera_name);
        std::this_thread::sleep_for(std::chrono::duration<double>(impl_->exposure_time));

        std::string result = "Exposure result with " + std::to_string(impl_->exposure_time) + " seconds.";
        co_return result;
    } catch (const std::exception& e) {
        throw std::runtime_error("Exposure failed: " + std::string(e.what()));
    }
    co_return "Exposure failed.";
}

auto SmartExposure::handleExposureError() -> TaskScheduler::Task {
    int retryAttempts = impl_->config_manager->getValue("/camera/retry_attempts")->get<int>();
    double qualityThreshold = impl_->config_manager->getValue("/camera/quality_threshold")->get<double>();

    for (int i = 0; i < retryAttempts; ++i) {
        try {
            LOG_F(INFO, "Retry attempt {} for camera {}", i + 1, impl_->camera_name);
            auto exposureTask = takeExposure();
            std::string result;
            co_await exposureTask;
            double quality = evaluateExposureQuality(result);

            if (quality >= qualityThreshold) {
                co_return result;
            } else {
                impl_->exposure_time = adjustExposureTime(impl_->exposure_time, quality);
            }
        } catch (const std::exception& e) {
            LOG_F(INFO, "Exposure attempt {} failed: {}", i + 1, e.what());
        }
    }
    co_return "Exposure failed after retries.";
}

auto SmartExposure::run() -> TaskScheduler::Task {
    auto validateTask = std::make_shared<TaskScheduler::Task>(validateExposure());
    impl_->task_scheduler->schedule("validate_exposure", validateTask);
    co_await *validateTask;

    auto exposureTask = std::make_shared<TaskScheduler::Task>(handleExposureError());
    impl_->task_scheduler->schedule("handle_exposure", exposureTask);
    co_await *exposureTask;

    co_return "Exposure completed";
}

auto SmartExposure::evaluateExposureQuality(const std::string& exposure_result) -> double {
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    return dis(gen);
}

auto SmartExposure::adjustExposureTime(double current_time, double quality) -> int {
    if (quality < QUALITY_THRESHOLD_LOW) {
        return static_cast<int>(current_time + 2);
    } else if (quality < QUALITY_THRESHOLD_HIGH) {
        return static_cast<int>(current_time + 1);
    } else if (quality > QUALITY_THRESHOLD_VERY_HIGH) {
        return static_cast<int>(std::max(1.0, current_time - 1));
    }
    return static_cast<int>(current_time);
}

}  // namespace lithium