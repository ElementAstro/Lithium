#ifndef LITHIUM_SEARCH_CROODS_HPP
#define LITHIUM_SEARCH_CROODS_HPP

#include <optional>
#include <vector>

#include "macro.hpp"

namespace lithium::tools {
struct CartesianCoordinates {
    double x;
    double y;
    double z;
} ATOM_ALIGNAS(32);

struct SphericalCoordinates {
    double rightAscension;
    double declination;
} ATOM_ALIGNAS(16);

struct MinMaxFOV {
    double minFOV;
    double maxFOV;
} ATOM_ALIGNAS(16);

auto rangeTo(double value, double max, double min) -> double;

auto degreeToRad(double degree) -> double;

auto radToDegree(double rad) -> double;

auto hourToDegree(double hour) -> double;

auto hourToRad(double hour) -> double;

auto degreeToHour(double degree) -> double;

auto radToHour(double rad) -> double;

auto getHaDegree(double RA_radian, double LST_Degree) -> double;

void raDecToAltAz(double ha_radian, double dec_radian, double& alt_radian,
                  double& az_radian, double lat_radian);

auto raDecToAltAz(double ha_radian, double dec_radian,
                  double lat_radian) -> std::vector<double>;

auto periodBelongs(double value, double min, double max, double period,
                   bool minequ, bool maxequ) -> bool;

auto convertEquatorialToCartesian(double ra, double dec,
                                  double radius) -> CartesianCoordinates;

auto calculateVector(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& pointB)
    -> CartesianCoordinates;

auto calculatePointC(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& vectorV)
    -> CartesianCoordinates;

auto convertToSphericalCoordinates(const CartesianCoordinates& cartesianPoint)
    -> std::optional<SphericalCoordinates>;

auto calculateFOV(int focalLength, double cameraSizeWidth,
                  double cameraSizeHeight) -> MinMaxFOV;
}  // namespace lithium::tools

#endif
