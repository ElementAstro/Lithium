#ifndef SMART_EXPOSURE_HPP
#define SMART_EXPOSURE_HPP

#include <coroutine>
#include <memory>
#include <string>
#include "atom/type/json_fwd.hpp"
#include "task/custom/cotask.hpp"
#include "task/interface/task.hpp"

using json = nlohmann::json;

namespace lithium {

class SmartExposure {
public:
    SmartExposure(const json& params);

    // Function to run the full exposure sequence
    auto run() -> TaskScheduler::Task;

private:
    // Function to validate exposure settings
    auto validateExposure() -> TaskScheduler::Task;

    // Function to execute an exposure
    auto takeExposure() -> TaskScheduler::Task;

    // Function to handle errors and retry logic
    auto handleExposureError() -> TaskScheduler::Task;

    // Helper functions
    auto evaluateExposureQuality(const std::string& exposure_result) -> double;
    auto adjustExposureTime(double current_time, double quality) -> int;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif
