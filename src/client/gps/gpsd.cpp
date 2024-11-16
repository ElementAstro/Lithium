#include "gpsd.hpp"
#include <libnova/julian_day.h>
#include <libnova/libnova.h>
#include <libnova/sidereal_time.h>
#include <array>
#include <cmath>
#include <loguru.hpp>

// Define default port
constexpr int TIMEOUT_MS = 1000;
constexpr double LONGITUDE_OFFSET = 360.0;
constexpr double SIDEREAL_TIME_OFFSET = 2.529722222;
constexpr double HOURS_IN_DAY = 24.0;
constexpr double DEGREES_PER_HOUR = 15.0;

class GPSD::Impl {
public:
    Impl() { LOG_F(INFO, "GPSD instance created"); }

    ~Impl() {
        disconnect();
        LOG_F(INFO, "GPSD instance destroyed");
    }

    auto connect(const std::string& host, const std::string& port) -> bool {
        try {
            gps = std::make_unique<gpsmm>(host.c_str(), port.c_str());
            if (gps->stream(WATCH_ENABLE | WATCH_JSON) == nullptr) {
                LOG_F(ERROR, "Unable to connect to GPSD server %s:%s",
                      host.c_str(), port.c_str());
                throw std::runtime_error("Unable to connect to GPSD server");
            }
            LOG_F(INFO, "Successfully connected to GPSD server %s:%s",
                  host.c_str(), port.c_str());
            return true;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception occurred while connecting to GPSD: %s",
                  e.what());
            return false;
        }
    }

    auto disconnect() -> bool {
        if (gps) {
            gps.reset();
            LOG_F(INFO, "GPS disconnected successfully");
            return true;
        }
        LOG_F(WARNING, "GPS not connected");
        return false;
    }

    auto updateGPS() -> std::optional<GPSData> {
        if (!gps || !gps->waiting(TIMEOUT_MS)) {
            LOG_F(WARNING, "GPS data unavailable or wait timed out");
            return std::nullopt;
        }

        struct gps_data_t* gpsData;
        while (gps->waiting(0)) {
            gpsData = gps->read();
            if (gpsData == nullptr) {
                LOG_F(ERROR, "Error reading from GPSD");
                return std::nullopt;
            }
            LOG_F(INFO, "Read GPS data: latitude=%f, longitude=%f, altitude=%f",
                  gpsData->fix.latitude, gpsData->fix.longitude,
                  gpsData->fix.altitude);
        }

        if (gpsData->fix.mode < MODE_2D) {
            latestData.fixStatus = "NO FIX";
            LOG_F(WARNING, "No GPS fix");
            return std::nullopt;
        }

        latestData.fixStatus =
            (gpsData->fix.mode == MODE_3D) ? "3D FIX" : "2D FIX";
        latestData.latitude = gpsData->fix.latitude;
        latestData.longitude = gpsData->fix.longitude;
        if (latestData.longitude.value() < 0) {
            latestData.longitude =
                latestData.longitude.value() + LONGITUDE_OFFSET;
        }
        latestData.altitude =
            (gpsData->fix.mode == MODE_3D) ? gpsData->fix.altitude : 0;
        latestData.time =
            std::chrono::system_clock::from_time_t(gpsData->fix.time.tv_sec);

        LOG_F(INFO, "GPS data updated successfully");
        return latestData;
    }

    [[nodiscard]] auto getLatitude() const -> std::optional<double> {
        return latestData.latitude;
    }

    [[nodiscard]] auto getLongitude() const -> std::optional<double> {
        return latestData.longitude;
    }

    [[nodiscard]] auto getAltitude() const -> std::optional<double> {
        return latestData.altitude;
    }

    [[nodiscard]] auto getTime() const
        -> std::optional<std::chrono::system_clock::time_point> {
        return latestData.time;
    }

    [[nodiscard]] auto getFixStatus() const -> std::optional<std::string> {
        return latestData.fixStatus;
    }

    [[nodiscard]] auto getPolarisHourAngle() const -> std::optional<double> {
        return latestData.polarisHourAngle;
    }

    // New feature implementation
    [[nodiscard]] auto getDevice() const -> std::optional<std::string> {
        if (!gps) {
            LOG_F(WARNING, "GPS not connected, unable to get device info");
            return std::nullopt;
        }
        struct gps_data_t* gpsData = gps->read();
        if (gpsData) {
            return std::string(gpsData->dev.path);
        }
        LOG_F(WARNING, "Unable to get device info");
        return std::nullopt;
    }

private:
    std::unique_ptr<gpsmm> gps;
    GPSData latestData;
};

GPSD::GPSD() : pImpl(std::make_unique<Impl>()) {}

GPSD::~GPSD() = default;

auto GPSD::connect(const std::string& host, const std::string& port) -> bool {
    return pImpl->connect(host, port);
}

auto GPSD::disconnect() -> bool { return pImpl->disconnect(); }

auto GPSD::updateGPS() -> std::optional<GPSData> { return pImpl->updateGPS(); }

auto GPSD::getLatitude() const -> std::optional<double> {
    return pImpl->getLatitude();
}

auto GPSD::getLongitude() const -> std::optional<double> {
    return pImpl->getLongitude();
}

auto GPSD::getAltitude() const -> std::optional<double> {
    return pImpl->getAltitude();
}

auto GPSD::getTime() const
    -> std::optional<std::chrono::system_clock::time_point> {
    return pImpl->getTime();
}

auto GPSD::getFixStatus() const -> std::optional<std::string> {
    return pImpl->getFixStatus();
}

auto GPSD::getDevice() const -> std::optional<std::string> {
    return pImpl->getDevice();
}