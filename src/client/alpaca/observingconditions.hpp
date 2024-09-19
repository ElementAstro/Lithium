#pragma once

#include <optional>
#include <string>
#include <string_view>
#include "device.hpp"

class AlpacaObservingConditions : public AlpacaDevice {
public:
    AlpacaObservingConditions(std::string_view address, int device_number,
                              std::string_view protocol = "http");
    ~AlpacaObservingConditions() override = default;

    // Properties
    auto getAveragePeriod() const -> double;
    void setAveragePeriod(double period);
    auto getCloudCover() const -> std::optional<double>;
    auto getDewPoint() const -> std::optional<double>;
    auto getHumidity() const -> std::optional<double>;
    auto getPressure() const -> std::optional<double>;
    auto getRainRate() const -> std::optional<double>;
    auto getSkyBrightness() const -> std::optional<double>;
    auto getSkyQuality() const -> std::optional<double>;
    auto getSkyTemperature() const -> std::optional<double>;
    auto getStarFWHM() const -> std::optional<double>;
    auto getTemperature() const -> std::optional<double>;
    auto getWindDirection() const -> std::optional<double>;
    auto getWindGust() const -> std::optional<double>;
    auto getWindSpeed() const -> std::optional<double>;

    // Methods
    void refresh();
    auto sensorDescription(std::string_view sensorName) const -> std::string;
    auto timeSinceLastUpdate(std::string_view sensorName) const -> double;

private:
    template <typename T>
    auto getOptionalProperty(const std::string& property) const
        -> std::optional<T> {
        try {
            return getNumericProperty<T>(property);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
};