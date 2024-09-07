#include "libastro.hpp"
#include <cmath>
#include <numbers>

namespace lithium {

namespace {

// Utility functions for internal use
double getObliquity(double jd) {
    double t = (jd - JD2000) / 36525.0;
    return 23.439291 - 0.0130042 * t - 1.64e-7 * t * t + 5.04e-7 * t * t * t;
}

}  // anonymous namespace

std::tuple<double, double> getNutation(double jd) {
    double t = (jd - JD2000) / 36525.0;
    double omega =
        125.04452 - 1934.136261 * t + 0.0020708 * t * t + t * t * t / 450000;
    double L = 280.4665 + 36000.7698 * t;
    double Ls = 218.3165 + 481267.8813 * t;

    double nutation_lon = -17.2 * std::sin(degToRad(omega)) -
                          1.32 * std::sin(2 * degToRad(L)) -
                          0.23 * std::sin(2 * degToRad(Ls)) +
                          0.21 * std::sin(2 * degToRad(omega));
    double nutation_obl =
        9.2 * std::cos(degToRad(omega)) + 0.57 * std::cos(2 * degToRad(L)) +
        0.1 * std::cos(2 * degToRad(Ls)) - 0.09 * std::cos(2 * degToRad(omega));

    return {nutation_lon / 3600.0, nutation_obl / 3600.0};
}

EquatorialCoordinates applyNutation(const EquatorialCoordinates& position,
                                    double jd, bool reverse) {
    auto [nutation_lon, nutation_obl] = getNutation(jd);
    double obliquity = degToRad(getObliquity(jd));

    double ra = degToRad(position.rightAscension * 15);
    double dec = degToRad(position.declination);

    double sign = reverse ? -1 : 1;

    double delta_ra = (std::cos(obliquity) +
                       std::sin(obliquity) * std::sin(ra) * std::tan(dec)) *
                          nutation_lon -
                      (std::cos(ra) * std::tan(dec)) * nutation_obl;
    double delta_dec = (std::sin(obliquity) * std::cos(ra)) * nutation_lon +
                       std::sin(ra) * nutation_obl;

    return {radToDeg(ra + sign * degToRad(delta_ra)) / 15.0,
            radToDeg(dec + sign * degToRad(delta_dec))};
}

EquatorialCoordinates applyAberration(const EquatorialCoordinates& position,
                                      double jd) {
    double t = (jd - JD2000) / 36525.0;
    double e = 0.016708634 - 0.000042037 * t - 0.0000001267 * t * t;
    double pi = 102.93735 + 1.71946 * t + 0.00046 * t * t;
    double lon = 280.46646 + 36000.77983 * t + 0.0003032 * t * t;

    double ra = degToRad(position.rightAscension * 15);
    double dec = degToRad(position.declination);

    double k = 20.49552 / 3600.0;  // Constant of aberration

    double delta_ra =
        -k *
        (std::cos(ra) * std::cos(degToRad(lon)) * std::cos(degToRad(pi)) +
         std::sin(ra) * std::sin(degToRad(lon))) /
        std::cos(dec);
    double delta_dec =
        -k * (std::sin(degToRad(pi)) *
              (std::sin(dec) * std::cos(degToRad(lon)) -
               std::cos(dec) * std::sin(ra) * std::sin(degToRad(lon))));

    return {radToDeg(ra + degToRad(delta_ra)) / 15.0,
            radToDeg(dec + degToRad(delta_dec))};
}

EquatorialCoordinates applyPrecession(const EquatorialCoordinates& position,
                                      double fromJD, double toJD) {
    double t = (fromJD - JD2000) / 36525.0;
    double T = (toJD - fromJD) / 36525.0;

    double zeta = (2306.2181 + 1.39656 * t - 0.000139 * t * t) * T +
                  (0.30188 - 0.000344 * t) * T * T + 0.017998 * T * T * T;
    double z = (2306.2181 + 1.39656 * t - 0.000139 * t * t) * T +
               (1.09468 + 0.000066 * t) * T * T + 0.018203 * T * T * T;
    double theta = (2004.3109 - 0.85330 * t - 0.000217 * t * t) * T -
                   (0.42665 + 0.000217 * t) * T * T - 0.041833 * T * T * T;

    zeta = degToRad(zeta / 3600.0);
    z = degToRad(z / 3600.0);
    theta = degToRad(theta / 3600.0);

    double ra = degToRad(position.rightAscension * 15);
    double dec = degToRad(position.declination);

    double A = std::cos(dec) * std::sin(ra + zeta);
    double B = std::cos(theta) * std::cos(dec) * std::cos(ra + zeta) -
               std::sin(theta) * std::sin(dec);
    double C = std::sin(theta) * std::cos(dec) * std::cos(ra + zeta) +
               std::cos(theta) * std::sin(dec);

    double ra_new = std::atan2(A, B) + z;
    double dec_new = std::asin(C);

    return {radToDeg(ra_new) / 15.0, radToDeg(dec_new)};
}

EquatorialCoordinates observedToJ2000(const EquatorialCoordinates& observed,
                                      double jd) {
    auto temp = applyAberration(observed, jd);
    temp = applyNutation(temp, jd, true);
    return applyPrecession(temp, jd, JD2000);
}

EquatorialCoordinates j2000ToObserved(const EquatorialCoordinates& j2000,
                                      double jd) {
    auto temp = applyPrecession(j2000, JD2000, jd);
    temp = applyNutation(temp, jd);
    return applyAberration(temp, jd);
}

HorizontalCoordinates equatorialToHorizontal(
    const EquatorialCoordinates& object, const GeographicCoordinates& observer,
    double jd) {
    double lst = range360(280.46061837 + 360.98564736629 * (jd - 2451545.0) +
                          observer.longitude);
    double ha = range360(lst - object.rightAscension * 15);

    double sin_alt = std::sin(degToRad(object.declination)) *
                         std::sin(degToRad(observer.latitude)) +
                     std::cos(degToRad(object.declination)) *
                         std::cos(degToRad(observer.latitude)) *
                         std::cos(degToRad(ha));
    double alt = radToDeg(std::asin(sin_alt));

    double cos_az =
        (std::sin(degToRad(object.declination)) -
         std::sin(degToRad(alt)) * std::sin(degToRad(observer.latitude))) /
        (std::cos(degToRad(alt)) * std::cos(degToRad(observer.latitude)));
    double az = radToDeg(std::acos(cos_az));

    if (std::sin(degToRad(ha)) > 0) {
        az = 360 - az;
    }

    return {range360(az + 180), alt};
}

EquatorialCoordinates horizontalToEquatorial(
    const HorizontalCoordinates& object, const GeographicCoordinates& observer,
    double jd) {
    double alt = degToRad(object.altitude);
    double az = degToRad(range360(object.azimuth + 180));
    double lat = degToRad(observer.latitude);

    double sin_dec = std::sin(alt) * std::sin(lat) +
                     std::cos(alt) * std::cos(lat) * std::cos(az);
    double dec = radToDeg(std::asin(sin_dec));

    double cos_ha = (std::sin(alt) - std::sin(lat) * std::sin(degToRad(dec))) /
                    (std::cos(lat) * std::cos(degToRad(dec)));
    double ha = radToDeg(std::acos(cos_ha));

    if (std::sin(az) > 0) {
        ha = 360 - ha;
    }

    double lst = range360(280.46061837 + 360.98564736629 * (jd - 2451545.0) +
                          observer.longitude);
    double ra = range360(lst - ha) / 15.0;

    return {ra, dec};
}

}  // namespace lithium
