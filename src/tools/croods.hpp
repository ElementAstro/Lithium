#ifndef LITHIUM_SEARCH_CROODS_HPP
#define LITHIUM_SEARCH_CROODS_HPP

#include <cmath>
#include <format>
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

template <std::floating_point T>
constexpr T LUMEN(T wavelength) {
    return 1.464128843e-3 / (wavelength * wavelength);
}

template <std::floating_point T>
constexpr T REDSHIFT(T observed, T rest) {
    return (observed - rest) / rest;
}

template <std::floating_point T>
constexpr T DOPPLER(T redshift, T speed) {
    return redshift * speed;
}

template <std::floating_point T>
T rangeHA(T r) {
    while (r < -12.0)
        r += 24.0;
    while (r >= 12.0)
        r -= 24.0;
    return r;
}

template <std::floating_point T>
T range24(T r) {
    while (r < 0.0)
        r += 24.0;
    while (r > 24.0)
        r -= 24.0;
    return r;
}

template <std::floating_point T>
T range360(T r) {
    while (r < 0.0)
        r += 360.0;
    while (r > 360.0)
        r -= 360.0;
    return r;
}

template <std::floating_point T>
T rangeDec(T decdegrees) {
    if ((decdegrees >= 270.0) && (decdegrees <= 360.0))
        return (decdegrees - 360.0);
    if ((decdegrees >= 180.0) && (decdegrees < 270.0))
        return (180.0 - decdegrees);
    if ((decdegrees >= 90.0) && (decdegrees < 180.0))
        return (180.0 - decdegrees);
    return decdegrees;
}

template <std::floating_point T>
T get_local_hour_angle(T sideral_time, T ra) {
    T HA = sideral_time - ra;
    return rangeHA(HA);
}

template <std::floating_point T>
std::pair<T, T> get_alt_az_coordinates(T Ha, T Dec, T Lat) {
    using namespace std::numbers;
    Ha *= pi_v<T> / 180.0;
    Dec *= pi_v<T> / 180.0;
    Lat *= pi_v<T> / 180.0;
    T alt = std::asin(std::sin(Dec) * std::sin(Lat) +
                      std::cos(Dec) * std::cos(Lat) * std::cos(Ha));
    T az = std::acos((std::sin(Dec) - std::sin(alt) * std::sin(Lat)) /
                     (std::cos(alt) * std::cos(Lat)));
    alt *= 180.0 / pi_v<T>;
    az *= 180.0 / pi_v<T>;
    if (std::sin(Ha) >= 0.0)
        az = 360 - az;
    return {alt, az};
}

template <std::floating_point T>
T estimate_geocentric_elevation(T Lat, T El) {
    using namespace std::numbers;
    Lat *= pi_v<T> / 180.0;
    Lat = std::sin(Lat);
    El += Lat * (EARTHRADIUSPOLAR - EARTHRADIUSEQUATORIAL);
    return El;
}

template <std::floating_point T>
T estimate_field_rotation_rate(T Alt, T Az, T Lat) {
    using namespace std::numbers;
    Alt *= pi_v<T> / 180.0;
    Az *= pi_v<T> / 180.0;
    Lat *= pi_v<T> / 180.0;
    T ret = std::cos(Lat) * std::cos(Az) / std::cos(Alt);
    ret *= 180.0 / pi_v<T>;
    return ret;
}

template <std::floating_point T>
T estimate_field_rotation(T HA, T rate) {
    HA *= rate;
    while (HA >= 360.0)
        HA -= 360.0;
    while (HA < 0)
        HA += 360.0;
    return HA;
}

template <std::floating_point T>
constexpr T as2rad(T as) {
    using namespace std::numbers;
    return as * pi_v<T> / (60.0 * 60.0 * 12.0);
}

template <std::floating_point T>
constexpr T rad2as(T rad) {
    using namespace std::numbers;
    return rad * (60.0 * 60.0 * 12.0) / pi_v<T>;
}

template <std::floating_point T>
T estimate_distance(T parsecs, T parallax_radius) {
    return parallax_radius / std::sin(as2rad(parsecs));
}

template <std::floating_point T>
constexpr T m2au(T m) {
    return m / ASTRONOMICALUNIT;
}

template <std::floating_point T>
T calc_delta_magnitude(T mag_ratio, std::span<const T> spectrum,
                       std::span<const T> ref_spectrum) {
    T delta_mag = 0;
    for (size_t l = 0; l < spectrum.size(); l++) {
        delta_mag += spectrum[l] * mag_ratio * ref_spectrum[l] / spectrum[l];
    }
    delta_mag /= spectrum.size();
    return delta_mag;
}

template <std::floating_point T>
T calc_star_mass(T delta_mag, T ref_size) {
    return delta_mag * ref_size;
}

template <std::floating_point T>
T estimate_orbit_radius(T obs_lambda, T ref_lambda, T period) {
    using namespace std::numbers;
    return pi_v<T> * 2 * DOPPLER(REDSHIFT(obs_lambda, ref_lambda), LIGHTSPEED) /
           period;
}

template <std::floating_point T>
T estimate_secondary_mass(T star_mass, T star_drift, T orbit_radius) {
    return orbit_radius * std::pow(star_drift * orbit_radius, 3) * 3 *
           star_mass;
}

template <std::floating_point T>
T estimate_secondary_size(T star_size, T dropoff_ratio) {
    return std::pow(dropoff_ratio * std::pow(star_size, 2), 0.5);
}

template <std::floating_point T>
T calc_photon_flux(T rel_magnitude, T filter_bandwidth, T wavelength,
                   T steradian) {
    return std::pow(10, rel_magnitude * -0.4) *
           (LUMEN(wavelength) * steradian * filter_bandwidth);
}

template <std::floating_point T>
T calc_rel_magnitude(T photon_flux, T filter_bandwidth, T wavelength,
                     T steradian) {
    return std::pow(10, 1.0 / (photon_flux / (LUMEN(wavelength) * steradian *
                                              filter_bandwidth))) /
           -0.4;
}

template <std::floating_point T>
T estimate_absolute_magnitude(T delta_dist, T delta_mag) {
    return std::sqrt(delta_dist) * delta_mag;
}

template <std::floating_point T>
std::array<T, 2> baseline_2d_projection(T alt, T az,
                                        const std::array<T, 3>& baseline,
                                        T wavelength) {
    using namespace std::numbers;
    az *= pi_v<T> / 180.0;
    alt *= pi_v<T> / 180.0;
    std::array<T, 2> uvresult;
    uvresult[0] = (baseline[0] * std::sin(az) + baseline[1] * std::cos(az));
    uvresult[1] = (baseline[1] * std::sin(alt) * std::sin(az) -
                   baseline[0] * std::sin(alt) * std::cos(az) +
                   baseline[2] * std::cos(alt));
    uvresult[0] *= AIRY / wavelength;
    uvresult[1] *= AIRY / wavelength;
    return uvresult;
}

template <std::floating_point T>
T baseline_delay(T alt, T az, const std::array<T, 3>& baseline) {
    using namespace std::numbers;
    az *= pi_v<T> / 180.0;
    alt *= pi_v<T> / 180.0;
    return std::cos(az) * baseline[1] * std::cos(alt) -
           baseline[0] * std::sin(az) * std::cos(alt) +
           std::sin(alt) * baseline[2];
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
struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
};

// 添加一个函数来计算儒略日
template <std::floating_point T>
T calculate_julian_date(const DateTime& dt) {
    int a = (14 - dt.month) / 12;
    int y = dt.year + 4800 - a;
    int m = dt.month + 12 * a - 3;

    T jd = dt.day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 -
           32045 + (dt.hour - 12) / 24.0 + dt.minute / 1440.0 +
           dt.second / 86400.0;

    return jd;
}

// 添加一个函数来计算恒星时
template <std::floating_point T>
T calculate_sidereal_time(const DateTime& dt, T longitude) {
    T jd = calculate_julian_date<T>(dt);
    T t = (jd - 2451545.0) / 36525.0;
    T theta = 280.46061837 + 360.98564736629 * (jd - 2451545.0) +
              0.000387933 * t * t - t * t * t / 38710000.0;

    theta = range360(theta);
    theta += longitude;

    return theta / 15.0;  // 转换为小时
}

// 添加一个函数来计算大气折射
template <std::floating_point T>
T calculate_refraction(T altitude, T temperature = 10.0, T pressure = 1010.0) {
    if (altitude < -0.5)
        return 0.0;  // 天体在地平线以下，不考虑折射

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

// 添加一个函数来计算视差
template <std::floating_point T>
CelestialCoords<T> apply_parallax(const CelestialCoords<T>& coords,
                                  const GeographicCoords<T>& observer,
                                  T distance, const DateTime& dt) {
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

// 添加一个函数来计算黄道坐标
template <std::floating_point T>
std::pair<T, T> equatorial_to_ecliptic(const CelestialCoords<T>& coords,
                                       T obliquity) {
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
T calculate_precession(const CelestialCoords<T>& coords, const DateTime& from,
                       const DateTime& to) {
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
std::string format_ra(T ra) {
    int hours = static_cast<int>(ra);
    int minutes = static_cast<int>((ra - hours) * 60);
    double seconds = ((ra - hours) * 60 - minutes) * 60;

    return std::format("{:02d}h {:02d}m {:.2f}s", hours, minutes, seconds);
}

// 添加一个函数来格式化赤纬
template <std::floating_point T>
std::string format_dec(T dec) {
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
