#pragma once

#include <optional>
#include <string>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"

// Struct to store GPS location
struct Location {
    double latitude;
    double longitude;
    double elevation;
};

// Custom exception classes
class GnssNoFixException : public std::runtime_error {
public:
    explicit GnssNoFixException(const std::string& message)
        : std::runtime_error("GNSS No Fix: " + message) {}
};

class GnssFailedToConnectException : public std::runtime_error {
public:
    explicit GnssFailedToConnectException(const std::string& message)
        : std::runtime_error("GNSS Failed to Connect: " + message) {}
};

class GpsClient {
public:
    explicit GpsClient(const std::string& eagleGpsUrl);
    ~GpsClient();

    std::optional<Location> getLocation();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
