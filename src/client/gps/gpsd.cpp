#include "gpsd.hpp"
#include <libnova/julian_day.h>
#include <libnova/sidereal_time.h>
#include <cmath>
#include <iostream>

GPSD::GPSD() = default;

GPSD::~GPSD() { disconnect(); }

bool GPSD::connect(const std::string& host, const std::string& port) {
    gps = std::make_unique<gpsmm>(host.c_str(), port.c_str());
    if (gps->stream(WATCH_ENABLE | WATCH_JSON) == nullptr) {
        std::cerr << "No GPSD running." << std::endl;
        return false;
    }
    return true;
}

bool GPSD::disconnect() {
    gps.reset();
    std::cout << "GPS disconnected successfully." << std::endl;
    return true;
}

std::optional<GPSData> GPSD::updateGPS() {
    if (!gps || !gps->waiting(1000)) {
        return std::nullopt;
    }

    struct gps_data_t* gpsData;
    while (gps->waiting(0)) {
        gpsData = gps->read();
        if (!gpsData) {
            std::cerr << "GPSD read error." << std::endl;
            return std::nullopt;
        }
    }

    if (gpsData->fix.mode < MODE_2D) {
        latestData.fixStatus = "NO FIX";
        return std::nullopt;
    }

    latestData.fixStatus = (gpsData->fix.mode == MODE_3D) ? "3D FIX" : "2D FIX";
    latestData.latitude = gpsData->fix.latitude;
    latestData.longitude = gpsData->fix.longitude;
    if (latestData.longitude.value() < 0) {
        latestData.longitude = latestData.longitude.value() + 360;
    }
    latestData.altitude =
        (gpsData->fix.mode == MODE_3D) ? gpsData->fix.altitude : 0;
    latestData.time =
        std::chrono::system_clock::from_time_t(gpsData->fix.time.tv_sec);
    latestData.polarisHourAngle = calculatePolarisHourAngle(gpsData);

    return latestData;
}

double GPSD::calculatePolarisHourAngle(const gps_data_t* gpsData) {
    double jd = ln_get_julian_from_timet(
        reinterpret_cast<__time_t*>(&gpsData->fix.time.tv_sec));

    double lst = ln_get_apparent_sidereal_time(jd);
    return std::fmod(lst - 2.529722222 + (gpsData->fix.longitude / 15.0), 24.0);
}

std::optional<double> GPSD::getLatitude() const { return latestData.latitude; }

std::optional<double> GPSD::getLongitude() const {
    return latestData.longitude;
}

std::optional<double> GPSD::getAltitude() const { return latestData.altitude; }

std::optional<std::chrono::system_clock::time_point> GPSD::getTime() const {
    return latestData.time;
}

std::optional<std::string> GPSD::getFixStatus() const {
    return latestData.fixStatus;
}

std::optional<double> GPSD::getPolarisHourAngle() const {
    return latestData.polarisHourAngle;
}