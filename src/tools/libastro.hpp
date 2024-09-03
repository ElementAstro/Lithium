#pragma once

#include <cmath>
#include <numbers>
#include <tuple>

#include "macro.hpp"

namespace lithium {

constexpr double JD2000 = 2451545.0;

struct EquatorialCoordinates {
    double rightAscension;  // in hours
    double declination;     // in degrees
} ATOM_ALIGNAS(16);

struct HorizontalCoordinates {
    double azimuth;   // in degrees
    double altitude;  // in degrees
} ATOM_ALIGNAS(16);

struct GeographicCoordinates {
    double longitude;  // in degrees
    double latitude;   // in degrees
    double elevation;  // in meters
} ATOM_ALIGNAS(32);

// Convert degrees to radians
constexpr double degToRad(double deg) { return deg * std::numbers::pi / 180.0; }

// Convert radians to degrees
constexpr double radToDeg(double rad) { return rad * 180.0 / std::numbers::pi; }

// Range 0 to 360
constexpr double range360(double angle) {
    return std::fmod(angle, 360.0) + (angle < 0 ? 360.0 : 0.0);
}

EquatorialCoordinates observedToJ2000(const EquatorialCoordinates& observed,
                                      double jd);
EquatorialCoordinates j2000ToObserved(const EquatorialCoordinates& j2000,
                                      double jd);
HorizontalCoordinates equatorialToHorizontal(
    const EquatorialCoordinates& object, const GeographicCoordinates& observer,
    double jd);
EquatorialCoordinates horizontalToEquatorial(
    const HorizontalCoordinates& object, const GeographicCoordinates& observer,
    double jd);

// Additional utility functions
std::tuple<double, double> getNutation(double jd);
EquatorialCoordinates applyNutation(const EquatorialCoordinates& position,
                                    double jd, bool reverse = false);
EquatorialCoordinates applyAberration(const EquatorialCoordinates& position,
                                      double jd);
EquatorialCoordinates applyPrecession(const EquatorialCoordinates& position,
                                      double fromJD, double toJD);

}  // namespace lithium
