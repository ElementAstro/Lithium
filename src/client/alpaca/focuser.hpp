#pragma once

#include <atomic>
#include <future>
#include <optional>
#include "device.hpp"

class AlpacaFocuser : public AlpacaDevice {
public:
    AlpacaFocuser(const std::string& address, int device_number,
                  const std::string& protocol = "http");
    ~AlpacaFocuser() override;

    // Properties
    auto getAbsolute() const -> bool;
    auto getIsMoving() const -> bool;
    auto getMaxIncrement() const -> int;
    auto getMaxStep() const -> int;
    auto getPosition() const -> int;
    auto getStepSize() const -> float;
    auto getTempComp() const -> bool;
    void setTempComp(bool tempCompState);
    auto getTempCompAvailable() const -> bool;
    auto getTemperature() const -> std::optional<float>;

    // Methods
    void halt();
    auto move(int position) -> std::future<void>;

    // Disable copy and move constructors and assignment operators
    AlpacaFocuser(const AlpacaFocuser&) = delete;
    AlpacaFocuser& operator=(const AlpacaFocuser&) = delete;
    AlpacaFocuser(AlpacaFocuser&&) = delete;
    AlpacaFocuser& operator=(AlpacaFocuser&&) = delete;

private:
    void startMove(int position);
    void moveThread(int position);

    std::atomic<bool> isMoving_{false};
    std::thread moveThread_;
};