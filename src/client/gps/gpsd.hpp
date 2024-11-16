#pragma once

#include <libgpsmm.h>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

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

    bool connect(const std::string& host = "localhost",
                 const std::string& port = DEFAULT_GPSD_PORT);
    bool disconnect();

    std::optional<GPSData> updateGPS();

    // Getter methods for GPS data
    std::optional<double> getLatitude() const;
    std::optional<double> getLongitude() const;
    std::optional<double> getAltitude() const;
    std::optional<std::chrono::system_clock::time_point> getTime() const;
    std::optional<std::string> getFixStatus() const;

    // 新增功能
    std::optional<std::string> getDevice() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // 禁止拷贝和赋值
    GPSD(const GPSD&) = delete;
    GPSD& operator=(const GPSD&) = delete;
};