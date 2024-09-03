#include "covercalibrator.hpp"
#include <stdexcept>
#include <thread>

AlpacaCoverCalibrator::AlpacaCoverCalibrator(const std::string& address,
                                             int device_number,
                                             const std::string& protocol)
    : AlpacaDevice(address, "covercalibrator", device_number, protocol) {}

int AlpacaCoverCalibrator::GetBrightness() {
    return GetNumericProperty<int>("brightness");
}

AlpacaCoverCalibrator::CalibratorStatus
AlpacaCoverCalibrator::GetCalibratorState() {
    return static_cast<CalibratorStatus>(
        GetNumericProperty<int>("calibratorstate"));
}

AlpacaCoverCalibrator::CoverStatus AlpacaCoverCalibrator::GetCoverState() {
    return static_cast<CoverStatus>(GetNumericProperty<int>("coverstate"));
}

int AlpacaCoverCalibrator::GetMaxBrightness() {
    return GetNumericProperty<int>("maxbrightness");
}

template <typename Func>
std::future<void> AlpacaCoverCalibrator::AsyncOperation(
    Func&& func, const std::string& operationName) {
    return std::async(
        std::launch::async,
        [this, func = std::forward<Func>(func), operationName]() {
            if (m_current_operation.valid() &&
                m_current_operation.wait_for(std::chrono::seconds(0)) !=
                    std::future_status::ready) {
                throw std::runtime_error("Another operation is in progress");
            }

            func();

            // Poll the device state until the operation is complete
            while (true) {
                auto state = operationName == "calibrator"
                                 ? GetCalibratorState()
                                 : GetCoverState();
                if (state != CalibratorStatus::NotReady &&
                    state != CoverStatus::Moving) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
}

std::future<void> AlpacaCoverCalibrator::CalibratorOff() {
    return AsyncOperation([this]() { Put("calibratoroff"); }, "calibrator");
}

std::future<void> AlpacaCoverCalibrator::CalibratorOn(int BrightnessVal) {
    return AsyncOperation(
        [this, BrightnessVal]() {
            Put("calibratoron",
                {{"Brightness", std::to_string(BrightnessVal)}});
        },
        "calibrator");
}

std::future<void> AlpacaCoverCalibrator::CloseCover() {
    return AsyncOperation([this]() { Put("closecover"); }, "cover");
}

void AlpacaCoverCalibrator::HaltCover() { Put("haltcover"); }

std::future<void> AlpacaCoverCalibrator::OpenCover() {
    return AsyncOperation([this]() { Put("opencover"); }, "cover");
}