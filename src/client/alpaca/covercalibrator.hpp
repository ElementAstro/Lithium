#pragma once

#include <chrono>
#include <future>
#include <optional>

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
    virtual ~AlpacaCoverCalibrator() = default;

    // Properties
    int GetBrightness();
    CalibratorStatus GetCalibratorState();
    CoverStatus GetCoverState();
    int GetMaxBrightness();

    // Methods
    std::future<void> CalibratorOff();
    std::future<void> CalibratorOn(int BrightnessVal);
    std::future<void> CloseCover();
    void HaltCover();
    std::future<void> OpenCover();

private:
    template <typename Func>
    std::future<void> AsyncOperation(Func&& func,
                                     const std::string& operationName);

    std::future<void> m_current_operation;
};