#pragma once

#include <future>
#include "device.hpp"

class AlpacaCoverCalibrator : public AlpacaDevice {
public:
    enum class CalibratorStatus {
        NotPresent = 0,
        Off = 1,
        NotReady = 2,
        Ready = 3,
        Unknown = 4,
        Error = 5
    };

    enum class CoverStatus {
        NotPresent = 0,
        Closed = 1,
        Moving = 2,
        Open = 3,
        Unknown = 4,
        Error = 5
    };

    AlpacaCoverCalibrator(const std::string& address, int device_number,
                          const std::string& protocol = "http");
    ~AlpacaCoverCalibrator() override = default;

    // Properties
    auto getBrightness() -> int;
    auto getCalibratorState() -> CalibratorStatus;
    auto getCoverState() -> CoverStatus;
    auto getMaxBrightness() -> int;

    // Methods
    auto calibratorOff() -> std::future<void>;
    auto calibratorOn(int brightnessVal) -> std::future<void>;
    auto closeCover() -> std::future<void>;
    void haltCover();
    auto openCover() -> std::future<void>;

private:
    template <typename Func>
    auto asyncOperation(Func&& func,
                        const std::string& operationName) -> std::future<void>;

    std::future<void> currentOperation_;
};
