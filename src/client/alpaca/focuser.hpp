#pragma once

#include <atomic>
#include <future>
#include <optional>
#include "device.hpp"

class AlpacaFocuser : public AlpacaDevice {
public:
    AlpacaFocuser(const std::string& address, int device_number,
                  const std::string& protocol = "http");
    virtual ~AlpacaFocuser();

    // Properties
    bool GetAbsolute();
    bool GetIsMoving();
    int GetMaxIncrement();
    int GetMaxStep();
    int GetPosition();
    float GetStepSize();
    bool GetTempComp();
    void SetTempComp(bool TempCompState);
    bool GetTempCompAvailable();
    std::optional<float> GetTemperature();

    // Methods
    void Halt();
    std::future<void> Move(int Position);

    // Template method for numeric properties

private:
    void StartMove(int Position);
    void MoveThread(int Position);

    std::atomic<bool> m_is_moving{false};
    std::thread m_move_thread;
};
