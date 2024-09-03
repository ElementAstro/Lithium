#pragma once

#include <concepts>
#include <future>
#include <optional>
#include <string>
#include "device.hpp"

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

class AlpacaRotator : public AlpacaDevice {
public:
    AlpacaRotator(const std::string& address, int device_number,
                  const std::string& protocol = "http");
    virtual ~AlpacaRotator() = default;

    // Properties
    bool GetCanReverse();
    bool GetIsMoving();
    double GetMechanicalPosition();
    double GetPosition();
    bool GetReverse();
    void SetReverse(bool ReverseState);
    std::optional<double> GetStepSize();
    double GetTargetPosition();

    // Methods
    void Halt();
    std::future<void> Move(double Position);
    std::future<void> MoveAbsolute(double Position);
    std::future<void> MoveMechanical(double Position);
    void Sync(double Position);

private:
    template <Numeric T>
    std::future<void> AsyncMove(const std::string& method, T Position) {
        return std::async(std::launch::async, [this, method, Position]() {
            Put(method, {{"Position", std::to_string(Position)}});
            while (GetIsMoving()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
};