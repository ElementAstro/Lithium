#include "observingconditions.hpp"
#include <stdexcept>

AlpacaObservingConditions::AlpacaObservingConditions(std::string_view address,
                                                     int device_number,
                                                     std::string_view protocol)
    : AlpacaDevice(std::string(address), "observingconditions", device_number,
                   std::string(protocol)) {}

auto AlpacaObservingConditions::getAveragePeriod() const -> double {
    return getNumericProperty<double>("averageperiod");
}

void AlpacaObservingConditions::setAveragePeriod(double period) {
    put("averageperiod", {{"AveragePeriod", std::to_string(period)}});
}

auto AlpacaObservingConditions::getCloudCover() const -> std::optional<double> {
    return getOptionalProperty<double>("cloudcover");
}

auto AlpacaObservingConditions::getDewPoint() const -> std::optional<double> {
    return getOptionalProperty<double>("dewpoint");
}

auto AlpacaObservingConditions::getHumidity() const -> std::optional<double> {
    return getOptionalProperty<double>("humidity");
}

auto AlpacaObservingConditions::getPressure() const -> std::optional<double> {
    return getOptionalProperty<double>("pressure");
}

auto AlpacaObservingConditions::getRainRate() const -> std::optional<double> {
    return getOptionalProperty<double>("rainrate");
}

auto AlpacaObservingConditions::getSkyBrightness() const -> std::optional<double> {
    return getOptionalProperty<double>("skybrightness");
}

auto AlpacaObservingConditions::getSkyQuality() const -> std::optional<double> {
    return getOptionalProperty<double>("skyquality");
}

auto AlpacaObservingConditions::getSkyTemperature() const -> std::optional<double> {
    return getOptionalProperty<double>("skytemperature");
}

auto AlpacaObservingConditions::getStarFWHM() const -> std::optional<double> {
    return getOptionalProperty<double>("starfwhm");
}

auto AlpacaObservingConditions::getTemperature() const -> std::optional<double> {
    return getOptionalProperty<double>("temperature");
}

auto AlpacaObservingConditions::getWindDirection() const -> std::optional<double> {
    return getOptionalProperty<double>("winddirection");
}

auto AlpacaObservingConditions::getWindGust() const -> std::optional<double> {
    return getOptionalProperty<double>("windgust");
}

auto AlpacaObservingConditions::getWindSpeed() const -> std::optional<double> {
    return getOptionalProperty<double>("windspeed");
}

void AlpacaObservingConditions::refresh() {
    put("refresh");
}

auto AlpacaObservingConditions::sensorDescription(std::string_view sensorName) const -> std::string {
    return get("sensordescription", {{"SensorName", std::string(sensorName)}});
}

auto AlpacaObservingConditions::timeSinceLastUpdate(std::string_view sensorName) const -> double {
    return getNumericProperty<double>("timesincelastupdate");
}
