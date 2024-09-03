#pragma once

#include <memory>
#include <string>
#include <string_view>

class SkySafariController {
public:
    struct Coordinates {
        double ra;   // Right Ascension in hours
        double dec;  // Declination in degrees
    };

    struct GeographicCoordinates {
        double latitude;
        double longitude;
    };

    struct DateTime {
        int year, month, day, hour, minute, second;
        double utcOffset;
    };

    enum class SlewRate { GUIDE, CENTERING, FIND, MAX };
    enum class Direction { NORTH, SOUTH, EAST, WEST };

    SkySafariController();
    ~SkySafariController();

    // Prevent copying
    SkySafariController(const SkySafariController&) = delete;
    SkySafariController& operator=(const SkySafariController&) = delete;

    // Allow moving
    SkySafariController(SkySafariController&&) noexcept;
    SkySafariController& operator=(SkySafariController&&) noexcept;

    bool initialize(const std::string& host, int port);
    std::string processCommand(std::string_view command);

    void setTargetCoordinates(const Coordinates& coords);
    void setGeographicCoordinates(const GeographicCoordinates& coords);
    void setDateTime(const DateTime& dt);
    void setSlewRate(SlewRate rate);

    bool startSlew(Direction direction);
    bool stopSlew(Direction direction);
    bool park();
    bool unpark();

    Coordinates getCurrentCoordinates() const;
    GeographicCoordinates getGeographicCoordinates() const;
    DateTime getDateTime() const;
    SlewRate getSlewRate() const;
    bool isConnected() const;
    bool isParked() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};