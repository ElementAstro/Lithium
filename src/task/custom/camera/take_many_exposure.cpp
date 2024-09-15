#include "take_many_exposure.hpp"
#include "task/utils/macro.hpp"

#include <chrono>
#include <format>
#include <random>
#include <thread>

#include "config/configor.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

#include "error/exception.hpp"
#include "utils/constant.hpp"

namespace lithium {
class TakeManyExposure::Impl {
public:
    std::string camera_name_;
    double exposure_time_;

    int gain;
    int max_gain_;
    int min_gain_;

    int offset;
    int max_offset_;
    int min_offset_;

    std::shared_ptr<ConfigManager> config_manager_;
    std::shared_ptr<TaskScheduler> task_scheduler_;
};

TakeManyExposure::TakeManyExposure(const json& params) {
    GET_OR_CREATE_PTR(impl_->config_manager_, ConfigManager,
                      Constants::CONFIG_MANAGER);
    GET_OR_CREATE_PTR(impl_->task_scheduler_, TaskScheduler,
                      Constants::TASK_SCHEDULER);

    GET_PARAM_OR_THROW(params, "camera_name", impl_->camera_name_)
    GET_PARAM_OR_THROW(params, "exposure_time", impl_->exposure_time_)
    GET_PARAM_OR_THROW(params, "gain", impl_->gain)
    GET_PARAM_OR_THROW(params, "offset", impl_->offset)
}

TaskScheduler::Task TakeManyExposure::validateExposure() {
    if (impl_->exposure_time_ < 0 || impl_->exposure_time_ > 3600) {
        LOG_F(ERROR, "Invalid exposure time: {}", impl_->exposure_time_);
        THROW_INVALID_ARGUMENT("Exposure failed due to long exposure time: ",
                               impl_->exposure_time_);
    }

    GET_CONFIG_VALUE(impl_->config_manager_,
                     std::format("/camera/{}/gain/max", impl_->camera_name_),
                     int, max_gain_);
    GET_CONFIG_VALUE(impl_->config_manager_,
                     std::format("/camera/{}/gain/min", impl_->camera_name_),
                     int, min_gain_);
    GET_CONFIG_VALUE(
        impl_->config_manager_,
        std::format("/camera/{}/gain/default", impl_->camera_name_), int,
        default_gain_);
    if (impl_->gain < min_gain_ || impl_->gain > max_gain_) {
        LOG_F(ERROR, "Invalid gain: {}", impl_->gain);
        impl_->gain = default_gain_;
        //THROW_INVALID_ARGUMENT("Exposure failed due to invalid gain: ",
        //                       impl_->gain);
        co_return std::format("Exposure failed due to invalid gain: {}",
                              impl_->gain);
    }

    co_yield std::format("Validated exposure time for camera {}: {}",
                         impl_->camera_name_, impl_->exposure_time_);
    co_return "Validation successful for camera " + impl_->camera_name_;
}

TaskScheduler::Task TakeManyExposure::takeExposure() {
    try {
        LOG_F(INFO, "Taking exposure for camera {} with {} seconds.",
              camera_name_, exposure_time_);
        std::this_thread::sleep_for(
            std::chrono::duration<double>(exposure_time_));

        std::string result = "Exposure result for camera " + camera_name_ +
                             " with " + std::to_string(exposure_time_) +
                             " seconds.";
        co_yield "Exposure completed: " + result;
        co_return result;
    } catch (const std::exception& e) {
        THROW_UNLAWFUL_OPERATION("Exposure failed for camera " + camera_name_ +
                                 ": " + e.what());
    }
}

TaskScheduler::Task TakeManyExposure::handleExposureError() {
    int exposure_time = exposure_time_;

    GET_CONFIG_VALUE(config_manager_, "/camera/retry_attempts", int,
                     retryAttempts);
    GET_CONFIG_VALUE(config_manager_, "/camera/retry_delay", int, retryDelay);
    GET_CONFIG_VALUE(config_manager_, "/camera/quality_threshold", double,
                     qualityThreshold);

    for (int i = 0; i < retryAttempts; ++i) {
        try {
            co_yield "Attempting exposure for camera " + camera_name_ +
                " after " + std::to_string(i + 1) + " retry(ies).";

            auto exposure_task = takeExposure();
            co_await exposure_task;

            if (auto result = task_scheduler_->getResult(exposure_task)) {
                double quality = evaluateExposureQuality(*result);
                LOG_F(INFO, "Exposure quality for camera {}: {}", camera_name_,
                      quality);

                if (quality >= qualityThreshold) {
                    co_return *result;
                } else {
                    exposure_time = adjustExposureTime(exposure_time, quality);
                    LOG_F(INFO, "Adjusted exposure time for camera {}: {}",
                          camera_name_, exposure_time);
                }
            } else {
                LOG_F(ERROR,
                      "Exposure task completed but no result produced for {}",
                      camera_name_);
                THROW_UNLAWFUL_OPERATION(
                    "Exposure task completed but no result produced for "
                    "camera " +
                    camera_name_);
            }
        } catch (const atom::error::UnlawfulOperation& e) {
            LOG_F(ERROR, "Exposure attempt {} failed for camera {}: {}", i + 1,
                  camera_name_, e.what());
        }
    }
    co_return std::format("Exposure failed for camera {} after {} retries.",
                          camera_name_, retryAttempts);
}

TaskScheduler::Task TakeManyExposure::run() {
    auto validate_task =
        std::make_shared<TaskScheduler::Task>(validateExposure());
    task_scheduler_->schedule("validate_exposure_" + camera_name_,
                              validate_task);

    auto exposure_task =
        std::make_shared<TaskScheduler::Task>(handleExposureError());
    exposure_task->dependencies.push_back("validate_exposure_" + camera_name_);
    task_scheduler_->schedule("exposure_task_" + camera_name_, exposure_task);

    co_await *exposure_task;

    co_return "Exposure sequence completed for camera " + camera_name_;
}

double TakeManyExposure::evaluateExposureQuality(
    const std::string& exposure_result) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

int TakeManyExposure::adjustExposureTime(double current_time, double quality) {
    if (quality < 0.3) {
        return current_time + 2;
    } else if (quality < 0.7) {
        return current_time + 1;
    } else if (quality > 0.9) {
        return std::max(1.0, current_time - 1);
    }
    return current_time;
}

}  // namespace lithium
