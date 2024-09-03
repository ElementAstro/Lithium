#pragma once

#include <optional>
#include <string>
#include <string_view>
#include "device.hpp"

class AlpacaObservingConditions : public AlpacaDevice {
public:
    AlpacaObservingConditions(std::string_view address, int device_number,
                              std::string_view protocol = "http");
    virtual ~AlpacaObservingConditions() = default;

    // Properties
    double GetAveragePeriod();
    void SetAveragePeriod(double period);
    std::optional<double> GetCloudCover();
    std::optional<double> GetDewPoint();
    std::optional<double> GetHumidity();
    std::optional<double> GetPressure();
    std::optional<double> GetRainRate();
    std::optional<double> GetSkyBrightness();
    std::optional<double> GetSkyQuality();
    std::optional<double> GetSkyTemperature();
    std::optional<double> GetStarFWHM();
    std::optional<double> GetTemperature();
    std::optional<double> GetWindDirection();
    std::optional<double> GetWindGust();
    std::optional<double> GetWindSpeed();

    // Methods
    void Refresh();
    std::string SensorDescription(std::string_view SensorName);
    double TimeSinceLastUpdate(std::string_view SensorName);

private:
    template <typename T>
    std::optional<T> GetOptionalProperty(const std::string& property) const {
        try {
            return GetNumericProperty<T>(property);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
};