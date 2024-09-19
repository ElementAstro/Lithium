#include "covercalibrator.hpp"
#include <stdexcept>
#include <thread>
#include "device.hpp"

AlpacaCoverCalibrator::AlpacaCoverCalibrator(const std::string& address,
                                             int device_number,
                                             const std::string& protocol)
    : AlpacaDevice(address, "covercalibrator", device_number, protocol) {}

auto AlpacaCoverCalibrator::getBrightness() -> int {
    return getNumericProperty<int>("brightness");
}

auto AlpacaCoverCalibrator::getCalibratorState() -> CalibratorStatus {
    return static_cast<CalibratorStatus>(
        getNumericProperty<int>("calibratorstate"));
}

auto AlpacaCoverCalibrator::getCoverState() -> CoverStatus {
    return static_cast<CoverStatus>(getNumericProperty<int>("coverstate"));
}

auto AlpacaCoverCalibrator::getMaxBrightness() -> int {
    return getNumericProperty<int>("maxbrightness");
}

template <typename Func>
auto AlpacaCoverCalibrator::asyncOperation(
    Func&& func, const std::string& operationName) -> std::future<void> {
    return std::async(
        std::launch::async,
        [this, func = std::forward<Func>(func), operationName]() {
            if (currentOperation_.valid() &&
                currentOperation_.wait_for(std::chrono::seconds(0)) !=
                    std::future_status::ready) {
                THROW_ANOTHER_OPERATION("Another operation is in progress");
            }

            func();

            // Poll the device state until the operation is complete
            while (true) {
                if (operationName == "calibrator") {
                    auto state = getCalibratorState();
                    if (state != CalibratorStatus::NotReady) {
                        break;
                    }
                } else {
                    auto state = getCoverState();
                    if (state != CoverStatus::Moving) {
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
}

auto AlpacaCoverCalibrator::calibratorOff() -> std::future<void> {
    return asyncOperation([this]() { put("calibratoroff"); }, "calibrator");
}

auto AlpacaCoverCalibrator::calibratorOn(int brightnessVal)
    -> std::future<void> {
    return asyncOperation(
        [this, brightnessVal]() {
            put("calibratoron",
                {{"Brightness", std::to_string(brightnessVal)}});
        },
        "calibrator");
}

auto AlpacaCoverCalibrator::closeCover() -> std::future<void> {
    return asyncOperation([this]() { put("closecover"); }, "cover");
}

void AlpacaCoverCalibrator::haltCover() { put("haltcover"); }

auto AlpacaCoverCalibrator::openCover() -> std::future<void> {
    return asyncOperation([this]() { put("opencover"); }, "cover");
}