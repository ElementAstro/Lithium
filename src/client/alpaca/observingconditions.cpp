#include "observingconditions.hpp"
#include <stdexcept>

AlpacaObservingConditions::AlpacaObservingConditions(std::string_view address,
                                                     int device_number,
                                                     std::string_view protocol)
    : AlpacaDevice(std::string(address), "observingconditions", device_number,
                   std::string(protocol)) {}

double AlpacaObservingConditions::GetAveragePeriod() {
    return GetNumericProperty<double>("averageperiod");
}

void AlpacaObservingConditions::SetAveragePeriod(double period) {
    Put("averageperiod", {{"AveragePeriod", std::to_string(period)}});
}

std::optional<double> AlpacaObservingConditions::GetCloudCover() {
    return GetOptionalProperty<double>("cloudcover");
}

std::optional<double> AlpacaObservingConditions::GetDewPoint() {
    return GetOptionalProperty<double>("dewpoint");
}

std::optional<double> AlpacaObservingConditions::GetHumidity() {
    return GetOptionalProperty<double>("humidity");
}

std::optional<double> AlpacaObservingConditions::GetPressure() {
    return GetOptionalProperty<double>("pressure");
}

std::optional<double> AlpacaObservingConditions::GetRainRate() {
    return GetOptionalProperty<double>("rainrate");
}

std::optional<double> AlpacaObservingConditions::GetSkyBrightness() {
    return GetOptionalProperty<double>("skybrightness");
}

std::optional<double> AlpacaObservingConditions::GetSkyQuality() {
    return GetOptionalProperty<double>("skyquality");
}

std::optional<double> AlpacaObservingConditions::GetSkyTemperature() {
    return GetOptionalProperty<double>("skytemperature");
}

std::optional<double> AlpacaObservingConditions::GetStarFWHM() {
    return GetOptionalProperty<double>("starfwhm");
}

std::optional<double> AlpacaObservingConditions::GetTemperature() {
    return GetOptionalProperty<double>("temperature");
}

std::optional<double> AlpacaObservingConditions::GetWindDirection() {
    return GetOptionalProperty<double>("winddirection");
}

std::optional<double> AlpacaObservingConditions::GetWindGust() {
    return GetOptionalProperty<double>("windgust");
}

std::optional<double> AlpacaObservingConditions::GetWindSpeed() {
    return GetOptionalProperty<double>("windspeed");
}

void AlpacaObservingConditions::Refresh() { Put("refresh"); }

std::string AlpacaObservingConditions::SensorDescription(
    std::string_view SensorName) {
    return Get("sensordescription", {{"SensorName", std::string(SensorName)}});
}

double AlpacaObservingConditions::TimeSinceLastUpdate(
    std::string_view SensorName) {
    return GetNumericProperty<double>("timesincelastupdate");
}
