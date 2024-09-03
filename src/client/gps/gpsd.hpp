#pragma once

#include <libgpsmm.h>
#include <chrono>
#include <string>
#include <optional>
#include <memory>

struct GPSData {
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::optional<double> altitude;
    std::optional<std::chrono::system_clock::time_point> time;
    std::optional<std::string> fixStatus;
    std::optional<double> polarisHourAngle;
};

class GPSD {
public:
    GPSD();
    ~GPSD();

    bool connect(const std::string& host = "localhost", const std::string& port = DEFAULT_GPSD_PORT);
    bool disconnect();
    
    std::optional<GPSData> updateGPS();

    // Getter methods for GPS data
    std::optional<double> getLatitude() const;
    std::optional<double> getLongitude() const;
    std::optional<double> getAltitude() const;
    std::optional<std::chrono::system_clock::time_point> getTime() const;
    std::optional<std::string> getFixStatus() const;
    std::optional<double> getPolarisHourAngle() const;

private:
    std::unique_ptr<gpsmm> gps;
    GPSData latestData;
    
    double calculatePolarisHourAngle(const gps_data_t* gpsData);
};