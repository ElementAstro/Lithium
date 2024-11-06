#pragma once

#include <cmath>
#include <numbers>
#include <tuple>

namespace lithium::tools {

constexpr double JD2000 = 2451545.0;
constexpr double DEG_TO_RAD = std::numbers::pi / 180.0;
constexpr double RAD_TO_DEG = 180.0 / std::numbers::pi;
constexpr double FULL_CIRCLE_DEG = 360.0;

struct alignas(16) EquatorialCoordinates {
    double rightAscension;  // in hours
    double declination;     // in degrees
};

struct alignas(16) HorizontalCoordinates {
    double azimuth;   // in degrees
    double altitude;  // in degrees
};

struct alignas(32) GeographicCoordinates {
    double longitude;  // in degrees
    double latitude;   // in degrees
    double elevation;  // in meters
};

// Convert degrees to radians
constexpr auto degToRad(double deg) -> double { return deg * DEG_TO_RAD; }

// Convert radians to degrees
constexpr auto radToDeg(double rad) -> double { return rad * RAD_TO_DEG; }

// Range 0 to 360
constexpr auto range360(double angle) -> double {
    return std::fmod(angle, FULL_CIRCLE_DEG) +
           (angle < 0 ? FULL_CIRCLE_DEG : 0.0);
}

auto observedToJ2000(const EquatorialCoordinates& observed,
                     double julianDate) -> EquatorialCoordinates;
auto j2000ToObserved(const EquatorialCoordinates& j2000,
                     double julianDate) -> EquatorialCoordinates;
auto equatorialToHorizontal(const EquatorialCoordinates& object,
                            const GeographicCoordinates& observer,
                            double julianDate) -> HorizontalCoordinates;
auto horizontalToEquatorial(const HorizontalCoordinates& object,
                            const GeographicCoordinates& observer,
                            double julianDate) -> EquatorialCoordinates;

// Additional utility functions
auto getNutation(double julianDate) -> std::tuple<double, double>;
auto applyNutation(const EquatorialCoordinates& position, double julianDate,
                   bool reverse = false) -> EquatorialCoordinates;
auto applyAberration(const EquatorialCoordinates& position,
                     double julianDate) -> EquatorialCoordinates;
auto applyPrecession(const EquatorialCoordinates& position,
                     double fromJulianDate,
                     double toJulianDate) -> EquatorialCoordinates;

}  // namespace lithium
