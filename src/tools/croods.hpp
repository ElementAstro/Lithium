#ifndef LITHIUM_SEARCH_CROODS_HPP
#define LITHIUM_SEARCH_CROODS_HPP

#include <cmath>
#include <concepts>
#include <format>
#include <numbers>
#include <optional>
#include <span>
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

constexpr double EARTHRADIUSEQUATORIAL = 6378137.0;
constexpr double EARTHRADIUSPOLAR = 6356752.0;
constexpr double ASTRONOMICALUNIT = 1.495978707e11;
constexpr double LIGHTSPEED = 299792458.0;
constexpr double AIRY = 1.21966;
constexpr double SOLARMASS = 1.98847e30;
constexpr double SOLARRADIUS = 6.957e8;
constexpr double PARSEC = 3.0857e16;

constexpr auto lumen(double wavelength) -> double {
    constexpr double MAGIC_NUMBER = 1.464128843e-3;
    return MAGIC_NUMBER / (wavelength * wavelength);
}

constexpr auto redshift(double observed, double rest) -> double {
    return (observed - rest) / rest;
}

constexpr auto doppler(double redshift, double speed) -> double {
    return redshift * speed;
}

template <typename T>
auto rangeHA(T range) -> T {
    constexpr double MAGIC_NUMBER24 = 24.0;
    constexpr double MAGIC_NUMBER12 = 12.0;

    if (range < -MAGIC_NUMBER12) {
        range += MAGIC_NUMBER24;
    }
    while (range >= MAGIC_NUMBER12) {
        range -= MAGIC_NUMBER24;
    }
    return range;
}

template <typename T>
auto range24(T range) -> T {
    constexpr double MAGIC_NUMBER24 = 24.0;

    if (range < 0.0) {
        range += MAGIC_NUMBER24;
    }
    while (range > MAGIC_NUMBER24) {
        range -= MAGIC_NUMBER24;
    }
    return range;
}

template <typename T>
auto range360(T range) -> T {
    constexpr double MAGIC_NUMBER360 = 360.0;

    if (range < 0.0) {
        range += MAGIC_NUMBER360;
    }
    while (range > MAGIC_NUMBER360) {
        range -= MAGIC_NUMBER360;
    }
    return range;
}

template <typename T>
auto rangeDec(T decDegrees) -> T {
    constexpr double MAGIC_NUMBER360 = 360.0;
    constexpr double MAGIC_NUMBER180 = 180.0;
    constexpr double MAGIC_NUMBER90 = 90.0;
    constexpr double MAGIC_NUMBER270 = 270.0;

    if (decDegrees >= MAGIC_NUMBER270 && decDegrees <= MAGIC_NUMBER360) {
        return decDegrees - MAGIC_NUMBER360;
    }
    if (decDegrees >= MAGIC_NUMBER180 && decDegrees < MAGIC_NUMBER270) {
        return MAGIC_NUMBER180 - decDegrees;
    }
    if (decDegrees >= MAGIC_NUMBER90 && decDegrees < MAGIC_NUMBER180) {
        return MAGIC_NUMBER180 - decDegrees;
    }
    return decDegrees;
}

template <typename T>
auto getLocalHourAngle(T siderealTime, T rightAscension) -> T {
    T hourAngle = siderealTime - rightAscension;
    return range24(hourAngle);
}

template <typename T>
auto getAltAzCoordinates(T hourAngle, T declination,
                         T latitude) -> std::pair<T, T> {
    using namespace std::numbers;
    hourAngle *= pi_v<T> / 180.0;
    declination *= pi_v<T> / 180.0;
    latitude *= pi_v<T> / 180.0;

    T altitude = std::asin(std::sin(declination) * std::sin(latitude) +
                           std::cos(declination) * std::cos(latitude) *
                               std::cos(hourAngle));
    T azimuth = std::acos(
        (std::sin(declination) - std::sin(altitude) * std::sin(latitude)) /
        (std::cos(altitude) * std::cos(latitude)));

    altitude *= 180.0 / pi_v<T>;
    azimuth *= 180.0 / pi_v<T>;

    if (hourAngle > 0) {
        azimuth = 360 - azimuth;
    }

    return {altitude, azimuth};
}

template <typename T>
auto estimateGeocentricElevation(T latitude, T elevation) -> T {
    using namespace std::numbers;
    latitude *= pi_v<T> / 180.0;
    return elevation - (elevation * std::cos(latitude));
}

template <typename T>
auto estimateFieldRotationRate(T altitude, T azimuth, T latitude) -> T {
    using namespace std::numbers;
    altitude *= pi_v<T> / 180.0;
    azimuth *= pi_v<T> / 180.0;
    latitude *= pi_v<T> / 180.0;

    T rate = std::cos(latitude) * std::sin(azimuth) / std::cos(altitude);
    return rate * 180.0 / pi_v<T>;
}

template <typename T>
auto estimateFieldRotation(T hourAngle, T rate) -> T {
    constexpr double MAGIC_NUMBER360 = 360.0;

    while (hourAngle >= MAGIC_NUMBER360) {
        hourAngle -= MAGIC_NUMBER360;
    }
    while (hourAngle < 0) {
        hourAngle += MAGIC_NUMBER360;
    }
    return hourAngle * rate;
}

constexpr auto as2rad(double arcSeconds) -> double {
    using namespace std::numbers;
    constexpr double MAGIC_NUMBER = 60.0 * 60.0 * 12.0;
    return arcSeconds * pi_v<double> / MAGIC_NUMBER;
}

constexpr auto rad2as(double radians) -> double {
    using namespace std::numbers;
    constexpr double MAGIC_NUMBER = 60.0 * 60.0 * 12.0;
    return radians * MAGIC_NUMBER / pi_v<double>;
}

template <typename T>
auto estimateDistance(T parsecs, T parallaxRadius) -> T {
    return parsecs / parallaxRadius;
}

constexpr auto m2au(double meters) -> double {
    constexpr double MAGIC_NUMBER = 1.496e+11;
    return meters / MAGIC_NUMBER;
}

template <typename T>
auto calcDeltaMagnitude(T magnitudeRatio, std::span<const T> spectrum) -> T {
    T deltaMagnitude = 0;
    for (size_t index = 0; index < spectrum.size(); ++index) {
        deltaMagnitude += spectrum[index] * magnitudeRatio;
    }
    return deltaMagnitude;
}

template <typename T>
auto calcStarMass(T deltaMagnitude, T referenceSize) -> T {
    return referenceSize * std::pow(10, deltaMagnitude / -2.5);
}

template <typename T>
auto estimateOrbitRadius(T observedWavelength, T referenceWavelength,
                         T period) -> T {
    return (observedWavelength - referenceWavelength) / period;
}

template <typename T>
auto estimateSecondaryMass(T starMass, T starDrift, T orbitRadius) -> T {
    return starMass * std::pow(starDrift / orbitRadius, 2);
}

template <typename T>
auto estimateSecondarySize(T starSize, T dropoffRatio) -> T {
    return starSize * std::sqrt(dropoffRatio);
}

template <typename T>
auto calcPhotonFlux(T relativeMagnitude, T filterBandwidth, T wavelength,
                    T steradian) -> T {
    constexpr double MAGIC_NUMBER10 = 10;
    constexpr double MAGIC_NUMBER04 = 0.4;

    return std::pow(MAGIC_NUMBER10, relativeMagnitude * -MAGIC_NUMBER04) *
           filterBandwidth * wavelength * steradian;
}

template <typename T>
auto calcRelMagnitude(T photonFlux, T filterBandwidth, T wavelength,
                      T steradian) -> T {
    constexpr double MAGIC_NUMBER04 = 0.4;
    return std::log10(photonFlux /
                      (LUMEN(wavelength) * steradian * filterBandwidth)) /
           -MAGIC_NUMBER04;
}

template <typename T>
auto estimateAbsoluteMagnitude(T deltaDistance, T deltaMagnitude) -> T {
    return deltaMagnitude - 5 * (std::log10(deltaDistance) - 1);
}

template <typename T>
auto baseline2dProjection(T altitude, T azimuth) -> std::array<T, 2> {
    using namespace std::numbers;
    altitude *= pi_v<T> / 180.0;
    azimuth *= pi_v<T> / 180.0;

    T x = std::cos(altitude) * std::cos(azimuth);
    T y = std::cos(altitude) * std::sin(azimuth);

    return {x, y};
}

template <typename T>
auto baselineDelay(T altitude, T azimuth,
                   const std::array<T, 3>& baseline) -> T {
    using namespace std::numbers;
    altitude *= pi_v<T> / 180.0;
    azimuth *= pi_v<T> / 180.0;

    T delay = baseline[0] * std::cos(altitude) * std::cos(azimuth) +
              baseline[1] * std::cos(altitude) * std::sin(azimuth) +
              baseline[2] * std::sin(altitude);

    return delay;
}

// 定义一个表示天体坐标的结构体
template <std::floating_point T>
struct CelestialCoords {
    T ra;   // 赤经 (小时)
    T dec;  // 赤纬 (度)
};

// 定义一个表示地理坐标的结构体
template <std::floating_point T>
struct GeographicCoords {
    T latitude;
    T longitude;
};

// 添加一个日期时间结构体
struct alignas(32) DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
};

// 添加一个函数来计算儒略日
template <std::floating_point T>
auto calculateJulianDate(const DateTime& dateTime) -> T {
    constexpr int MAGIC_NUMBER12 = 12;
    constexpr int MAGIC_NUMBER24 = 24;
    constexpr int MAGIC_NUMBER1440 = 1440;
    constexpr int MAGIC_NUMBER86400 = 86400;
    constexpr int MAGIC_NUMBER32045 = 32045;
    constexpr int MAGIC_NUMBER4800 = 4800;
    constexpr int MAGIC_NUMBER14 = 14;
    constexpr int MAGIC_NUMBER5 = 5;
    constexpr int MAGIC_NUMBER153 = 153;
    constexpr int MAGIC_NUMBER365 = 365;
    constexpr int MAGIC_NUMBER100 = 100;
    constexpr int MAGIC_NUMBER400 = 400;

    int a = (MAGIC_NUMBER14 - dateTime.month) / MAGIC_NUMBER12;
    int y = dateTime.year + MAGIC_NUMBER4800 - a;
    int m = dateTime.month + MAGIC_NUMBER12 * a - 3;

    T julianDate = dateTime.day + (MAGIC_NUMBER153 * m + 2) / MAGIC_NUMBER5 +
                   MAGIC_NUMBER365 * y + y / 4 - y / MAGIC_NUMBER100 +
                   y / MAGIC_NUMBER400 - MAGIC_NUMBER32045 +
                   (dateTime.hour - MAGIC_NUMBER12) / MAGIC_NUMBER24 +
                   dateTime.minute / MAGIC_NUMBER1440 +
                   dateTime.second / MAGIC_NUMBER86400;

    return julianDate;
}

template <typename T>
auto calculateSiderealTime(const DateTime& dateTime, T /*longitude*/) -> T {
    constexpr double MAGIC_NUMBER2451545 = 2451545.0;
    constexpr double MAGIC_NUMBER36525 = 36525.0;
    constexpr double MAGIC_NUMBER28046061837 = 280.46061837;
    constexpr double MAGIC_NUMBER36098564736629 = 360.98564736629;
    constexpr double MAGIC_NUMBER0000387933 = 0.000387933;
    constexpr double MAGIC_NUMBER38710000 = 38710000.0;
    constexpr double MAGIC_NUMBER15 = 15.0;

    T julianDate = calculateJulianDate<T>(dateTime);
    T t = (julianDate - MAGIC_NUMBER2451545) / MAGIC_NUMBER36525;

    T theta = MAGIC_NUMBER28046061837 +
              MAGIC_NUMBER36098564736629 * (julianDate - MAGIC_NUMBER2451545) +
              MAGIC_NUMBER0000387933 * t * t - t * t * t / MAGIC_NUMBER38710000;

    return theta / MAGIC_NUMBER15;  // 转换为小时
}

// 添加一个函数来计算大气折射
template <std::floating_point T>
auto calculateRefraction(T altitude, T temperature = 10.0,
                         T pressure = 1010.0) -> T {
    if (altitude < -0.5) {
        return 0.0;  // 天体在地平线以下，不考虑折射
    }

    T R;
    if (altitude > 15.0) {
        R = 0.00452 * pressure /
            ((273 + temperature) *
             std::tan(altitude * std::numbers::pi / 180.0));
    } else {
        T a = altitude;
        T b = altitude + 7.31 / (altitude + 4.4);
        R = 0.1594 + 0.0196 * a + 0.00002 * a * a;
        R *= pressure * (1 - 0.00012 * (temperature - 10)) / 1010.0;
        R /= 60.0;
    }

    return R;
}

template <std::floating_point T>
auto applyParallax(const CelestialCoords<T>& coords,
                   const GeographicCoords<T>& observer, T distance,
                   const DateTime& dt) -> CelestialCoords<T> {
    T lst = calculate_sidereal_time(dt, observer.longitude);
    T ha = lst - coords.ra;

    T sinLat = std::sin(observer.latitude * std::numbers::pi / 180.0);
    T cosLat = std::cos(observer.latitude * std::numbers::pi / 180.0);
    T sinDec = std::sin(coords.dec * std::numbers::pi / 180.0);
    T cosDec = std::cos(coords.dec * std::numbers::pi / 180.0);
    T sinHA = std::sin(ha * std::numbers::pi / 12.0);
    T cosHA = std::cos(ha * std::numbers::pi / 12.0);

    T rho = EARTHRADIUSEQUATORIAL / (PARSEC * distance);

    T A = cosLat * sinHA;
    T B = sinLat * cosDec - cosLat * sinDec * cosHA;
    T C = sinLat * sinDec + cosLat * cosDec * cosHA;

    T newRA = coords.ra - std::atan2(A, C - rho) * 12.0 / std::numbers::pi;
    T newDec = std::atan2((B * (C - rho) + A * A * sinDec / cosDec) /
                              ((C - rho) * (C - rho) + A * A),
                          cosDec) *
               180.0 / std::numbers::pi;

    return {range24(newRA), rangeDec(newDec)};
}

template <std::floating_point T>
auto equatorialToEcliptic(const CelestialCoords<T>& coords,
                          T obliquity) -> std::pair<T, T> {
    T sinDec = std::sin(coords.dec * std::numbers::pi / 180.0);
    T cosDec = std::cos(coords.dec * std::numbers::pi / 180.0);
    T sinRA = std::sin(coords.ra * std::numbers::pi / 12.0);
    T cosRA = std::cos(coords.ra * std::numbers::pi / 12.0);
    T sinObl = std::sin(obliquity * std::numbers::pi / 180.0);
    T cosObl = std::cos(obliquity * std::numbers::pi / 180.0);

    T latitude = std::asin(sinDec * cosObl - cosDec * sinObl * sinRA) * 180.0 /
                 std::numbers::pi;
    T longitude =
        std::atan2(sinRA * cosDec * cosObl + sinDec * sinObl, cosDec * cosRA) *
        180.0 / std::numbers::pi;

    return {range360(longitude), latitude};
}

// 添加一个函数来计算前进
template <std::floating_point T>
auto calculatePrecession(const CelestialCoords<T>& coords, const DateTime& from,
                         const DateTime& to) -> T {
    auto jd1 = calculate_julian_date<T>(from);
    auto jd2 = calculate_julian_date<T>(to);

    T T1 = (jd1 - 2451545.0) / 36525.0;
    T t = (jd2 - jd1) / 36525.0;

    T zeta = (2306.2181 + 1.39656 * T1 - 0.000139 * T1 * T1) * t +
             (0.30188 - 0.000344 * T1) * t * t + 0.017998 * t * t * t;
    T z = (2306.2181 + 1.39656 * T1 - 0.000139 * T1 * T1) * t +
          (1.09468 + 0.000066 * T1) * t * t + 0.018203 * t * t * t;
    T theta = (2004.3109 - 0.85330 * T1 - 0.000217 * T1 * T1) * t -
              (0.42665 + 0.000217 * T1) * t * t - 0.041833 * t * t * t;

    zeta /= 3600.0;
    z /= 3600.0;
    theta /= 3600.0;

    T A = std::cos(coords.dec * std::numbers::pi / 180.0) *
          std::sin(coords.ra * std::numbers::pi / 12.0 +
                   zeta * std::numbers::pi / 180.0);
    T B = std::cos(theta * std::numbers::pi / 180.0) *
              std::cos(coords.dec * std::numbers::pi / 180.0) *
              std::cos(coords.ra * std::numbers::pi / 12.0 +
                       zeta * std::numbers::pi / 180.0) -
          std::sin(theta * std::numbers::pi / 180.0) *
              std::sin(coords.dec * std::numbers::pi / 180.0);
    T C = std::sin(theta * std::numbers::pi / 180.0) *
              std::cos(coords.dec * std::numbers::pi / 180.0) *
              std::cos(coords.ra * std::numbers::pi / 12.0 +
                       zeta * std::numbers::pi / 180.0) +
          std::cos(theta * std::numbers::pi / 180.0) *
              std::sin(coords.dec * std::numbers::pi / 180.0);

    T newRA = std::atan2(A, B) * 12.0 / std::numbers::pi + z / 15.0;
    T newDec = std::asin(C) * 180.0 / std::numbers::pi;

    return std::sqrt(std::pow(newRA - coords.ra, 2) +
                     std::pow(newDec - coords.dec, 2));
}

// 添加一个函数来格式化赤经
template <std::floating_point T>
auto formatRa(T ra) -> std::string {
    int hours = static_cast<int>(ra);
    int minutes = static_cast<int>((ra - hours) * 60);
    double seconds = ((ra - hours) * 60 - minutes) * 60;

    return std::format("{:02d}h {:02d}m {:.2f}s", hours, minutes, seconds);
}

// 添加一个函数来格式化赤纬
template <std::floating_point T>
auto formatDec(T dec) -> std::string {
    char sign = (dec >= 0) ? '+' : '-';
    dec = std::abs(dec);
    int degrees = static_cast<int>(dec);
    int minutes = static_cast<int>((dec - degrees) * 60);
    double seconds = ((dec - degrees) * 60 - minutes) * 60;

    return std::format("{}{:02d}° {:02d}' {:.2f}\"", sign, degrees, minutes,
                       seconds);
}

}  // namespace lithium::tools

#endif
