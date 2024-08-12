#include "croods.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace lithium::tools {

auto rangeTo(double value, double max, double min) -> double {
    double period = max - min;
    while (value < min) {
        value += period;
    }
    while (value > max) {
        value -= period;
    }
    return value;
}

auto degreeToRad(double degree) -> double { return M_PI * (degree / 180.0); }

auto radToDegree(double rad) -> double { return rad * 180.0 / M_PI; }

auto hourToDegree(double hour) -> double {
    double degree = hour * 15.0;
    return rangeTo(degree, 360.0, 0.0);
}

auto hourToRad(double hour) -> double {
    double degree = hour * 15.0;
    degree = rangeTo(degree, 360.0, 0.0);
    return degreeToRad(degree);
}

auto degreeToHour(double degree) -> double {
    double hour = degree / 15.0;
    return rangeTo(hour, 24.0, 0.0);
}

auto radToHour(double rad) -> double {
    double degree = radToDegree(rad);
    degree = rangeTo(degree, 360.0, 0.0);
    return degreeToHour(degree);
}

auto getHaDegree(double RA_radian, double LST_Degree) -> double {
    double ha = LST_Degree - radToDegree(RA_radian);
    return rangeTo(ha, 360.0, 0.0);
}

auto raDecToAltAz(double ha_radian, double dec_radian,
                  double lat_radian) -> std::vector<double> {
    double cosLat = std::cos(lat_radian);

    auto altRadian =
        std::asin(std::sin(lat_radian) * std::sin(dec_radian) +
                  cosLat * std::cos(dec_radian) * std::cos(ha_radian));
    double azRadian;
    if (cosLat < 1e-5) {
        azRadian = ha_radian;  // polar case
    } else {
        double temp = std::acos((std::sin(dec_radian) -
                                 std::sin(altRadian) * std::sin(lat_radian)) /
                                (std::cos(altRadian) * cosLat));
        if (std::sin(ha_radian) > 0) {
            azRadian = 2 * M_PI - temp;
        } else {
            azRadian = temp;
        }
    }
    return {altRadian, azRadian};
}

void altAzToRaDec(double alt_radian, double az_radian, double& hr_radian,
                  double& dec_radian, double lat_radian) {
    double cosLat = std::cos(lat_radian);
    if (alt_radian > M_PI / 2.0) {
        alt_radian = M_PI - alt_radian;
        az_radian += M_PI;
    }
    if (alt_radian < -M_PI / 2.0) {
        alt_radian = -M_PI - alt_radian;
        az_radian -= M_PI;
    }
    double sinDec = std::sin(lat_radian) * std::sin(alt_radian) +
                    cosLat * std::cos(alt_radian) * std::cos(az_radian);
    dec_radian = std::asin(sinDec);
    if (cosLat < 1e-5) {
        hr_radian = az_radian + M_PI;
    } else {
        double temp = cosLat * std::cos(dec_radian);
        temp = (std::sin(alt_radian) - std::sin(lat_radian) * sinDec) / temp;
        temp = std::acos(std::clamp(-temp, -1.0, 1.0));
        if (std::sin(az_radian) > 0.0) {
            hr_radian = M_PI + temp;
        } else {
            hr_radian = M_PI - temp;
        }
    }
}

auto periodBelongs(double value, double min, double max, double period,
                   bool minequ, bool maxequ) -> bool {
    int n = static_cast<int>((value - max) / period);
    double ranges[3][2] = {{min + (n - 1) * period, max + (n - 1) * period},
                           {min + n * period, max + n * period},
                           {min + (n + 1) * period, max + (n + 1) * period}};

    for (const auto& range : ranges) {
        if ((maxequ && minequ && (value >= range[0] && value <= range[1])) ||
            (maxequ && !minequ && (value > range[0] && value <= range[1])) ||
            (!maxequ && !minequ && (value > range[0] && value < range[1])) ||
            (!maxequ && minequ && (value >= range[0] && value < range[1]))) {
            return true;
        }
    }

    return false;
}

auto convertEquatorialToCartesian(double ra, double dec,
                                  double radius) -> CartesianCoordinates {
    double raRad = (ra / 180.0) * M_PI;
    double decRad = (dec / 180.0) * M_PI;

    double x = radius * std::cos(decRad) * std::cos(raRad);
    double y = radius * std::cos(decRad) * std::sin(raRad);
    double z = radius * std::sin(decRad);

    return {x, y, z};
}

auto calculateVector(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& pointB)
    -> CartesianCoordinates {
    return {pointB.x - pointA.x, pointB.y - pointA.y, pointB.z - pointA.z};
}

auto calculatePointC(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& vectorV)
    -> CartesianCoordinates {
    return {pointA.x + vectorV.x, pointA.y + vectorV.y, pointA.z + vectorV.z};
}

auto convertToSphericalCoordinates(const CartesianCoordinates& cartesianPoint)
    -> std::optional<SphericalCoordinates> {
    double x = cartesianPoint.x;
    double y = cartesianPoint.y;
    double z = cartesianPoint.z;

    double radius = std::sqrt(x * x + y * y + z * z);
    if (radius == 0.0) {
        return std::nullopt;
    }

    double declination = std::asin(z / radius) * (180.0 / M_PI);
    double rightAscension = std::atan2(y, x) * (180.0 / M_PI);

    if (rightAscension < 0) {
        rightAscension += 360;
    }

    return SphericalCoordinates{rightAscension, declination};
}

auto calculateFOV(int focalLength, double cameraSizeWidth,
                  double cameraSizeHeight) -> MinMaxFOV {
    double cameraSizeDiagonal = std::hypot(cameraSizeWidth, cameraSizeHeight);

    double minFOV =
        2 * std::atan(cameraSizeHeight / (2.0 * focalLength)) * 180.0 / M_PI;
    double maxFOV =
        2 * std::atan(cameraSizeDiagonal / (2.0 * focalLength)) * 180.0 / M_PI;

    return {minFOV, maxFOV};
}
}  // namespace lithium::tools