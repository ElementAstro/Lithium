#ifndef LITHIUM_TASK_CUSTOM_CAMERA_TAKE_MANY_EXPOSURE_HPP
#define LITHIUM_TASK_CUSTOM_CAMERA_TAKE_MANY_EXPOSURE_HPP

#include <memory>
#include <string>
#include "atom/type/json_fwd.hpp"
#include "task/custom/cotask.hpp"
#include "task/interface/task.hpp"

using json = nlohmann::json;

namespace lithium {

class TakeManyExposure : public ITask {
public:
    TakeManyExposure(const json& params);

    // Function to run the full exposure sequence
    auto run() -> TaskScheduler::Task override;

private:
    // Function to validate the exposure settings
    auto validateExposure() -> TaskScheduler::Task;

    // Function to execute an exposure
    auto takeExposure() -> TaskScheduler::Task;

    // Function to handle errors and retry logic
    auto handleExposureError() -> TaskScheduler::Task;

    // Utility functions
    auto evaluateExposureQuality(const std::string& exposure_result) -> double;
    auto adjustExposureTime(double current_time, double quality) -> int;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif
