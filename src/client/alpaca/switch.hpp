#pragma once

#include <concepts>
#include <span>
#include <string>
#include <vector>
#include "device.hpp"

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

class AlpacaSwitch : public AlpacaDevice {
public:
    AlpacaSwitch(const std::string& address, int device_number,
                 const std::string& protocol = "http");
    virtual ~AlpacaSwitch() = default;

    // Properties
    int GetMaxSwitch();

    // Methods
    bool CanWrite(int Id);
    bool GetSwitch(int Id);
    std::string GetSwitchDescription(int Id);
    std::string GetSwitchName(int Id);
    double GetSwitchValue(int Id);
    double MaxSwitchValue(int Id);
    double MinSwitchValue(int Id);
    void SetSwitch(int Id, bool State);
    void SetSwitchName(int Id, const std::string& Name);
    void SetSwitchValue(int Id, double Value);
    double SwitchStep(int Id);

private:
    template <Numeric T>
    T GetSwitchProperty(const std::string& property, int Id) const {
        return GetNumericProperty<T>(property, {{"Id", std::to_string(Id)}});
    }

    template <typename T>
    void SetSwitchProperty(const std::string& property, int Id,
                           const T& value) {
        if constexpr (std::is_same_v<T, bool>) {
            Put(property, {{"Id", std::to_string(Id)},
                           {"State", value ? "true" : "false"}});
        } else if constexpr (std::is_arithmetic_v<T>) {
            Put(property,
                {{"Id", std::to_string(Id)}, {"Value", std::to_string(value)}});
        } else if constexpr (std::is_same_v<T, std::string>) {
            Put(property, {{"Id", std::to_string(Id)}, {"Name", value}});
        }
    }
};
