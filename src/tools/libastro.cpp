#include "libastro.hpp"
#include <cmath>

namespace lithium::tools {

namespace {

// Utility functions for internal use
constexpr double CENTURY = 36525.0;
constexpr double ARCSEC_TO_DEG = 1.0 / 3600.0;
constexpr double OBLIQUITY_COEFF1 = 23.439291;
constexpr double OBLIQUITY_COEFF2 = -0.0130042;
constexpr double OBLIQUITY_COEFF3 = -1.64e-7;
constexpr double OBLIQUITY_COEFF4 = 5.04e-7;

auto getObliquity(double julianDate) -> double {
    double centuriesSinceJ2000 = (julianDate - JD2000) / CENTURY;
    return OBLIQUITY_COEFF1 + OBLIQUITY_COEFF2 * centuriesSinceJ2000 +
           OBLIQUITY_COEFF3 * centuriesSinceJ2000 * centuriesSinceJ2000 +
           OBLIQUITY_COEFF4 * centuriesSinceJ2000 * centuriesSinceJ2000 *
               centuriesSinceJ2000;
}

}  // anonymous namespace

auto getNutation(double julianDate) -> std::tuple<double, double> {
    double centuriesSinceJ2000 = (julianDate - JD2000) / CENTURY;
    double omega = 125.04452 - 1934.136261 * centuriesSinceJ2000 +
                   0.0020708 * centuriesSinceJ2000 * centuriesSinceJ2000 +
                   centuriesSinceJ2000 * centuriesSinceJ2000 *
                       centuriesSinceJ2000 / 450000;
    double meanLongitudeSun = 280.4665 + 36000.7698 * centuriesSinceJ2000;
    double meanLongitudeMoon = 218.3165 + 481267.8813 * centuriesSinceJ2000;

    double nutationLongitude =
        -17.2 * std::sin(degToRad(omega)) -
        1.32 * std::sin(2 * degToRad(meanLongitudeSun)) -
        0.23 * std::sin(2 * degToRad(meanLongitudeMoon)) +
        0.21 * std::sin(2 * degToRad(omega));
    double nutationObliquity = 9.2 * std::cos(degToRad(omega)) +
                               0.57 * std::cos(2 * degToRad(meanLongitudeSun)) +
                               0.1 * std::cos(2 * degToRad(meanLongitudeMoon)) -
                               0.09 * std::cos(2 * degToRad(omega));

    return {nutationLongitude * ARCSEC_TO_DEG,
            nutationObliquity * ARCSEC_TO_DEG};
}

auto applyNutation(const EquatorialCoordinates& position, double julianDate,
                   bool reverse) -> EquatorialCoordinates {
    auto [nutationLongitude, nutationObliquity] = getNutation(julianDate);
    double obliquity = degToRad(getObliquity(julianDate));

    double rightAscension = degToRad(position.rightAscension * 15);
    double declination = degToRad(position.declination);

    double sign = reverse ? -1 : 1;

    double deltaRightAscension =
        (std::cos(obliquity) + std::sin(obliquity) * std::sin(rightAscension) *
                                   std::tan(declination)) *
            nutationLongitude -
        (std::cos(rightAscension) * std::tan(declination)) * nutationObliquity;
    double deltaDeclination =
        (std::sin(obliquity) * std::cos(rightAscension)) * nutationLongitude +
        std::sin(rightAscension) * nutationObliquity;

    return {
        radToDeg(rightAscension + sign * degToRad(deltaRightAscension)) / 15.0,
        radToDeg(declination + sign * degToRad(deltaDeclination))};
}

auto applyAberration(const EquatorialCoordinates& position,
                     double julianDate) -> EquatorialCoordinates {
    double centuriesSinceJ2000 = (julianDate - JD2000) / CENTURY;
    double eccentricity =
        0.016708634 - 0.000042037 * centuriesSinceJ2000 -
        0.0000001267 * centuriesSinceJ2000 * centuriesSinceJ2000;
    double perihelionLongitude =
        102.93735 + 1.71946 * centuriesSinceJ2000 +
        0.00046 * centuriesSinceJ2000 * centuriesSinceJ2000;
    double meanLongitude =
        280.46646 + 36000.77983 * centuriesSinceJ2000 +
        0.0003032 * centuriesSinceJ2000 * centuriesSinceJ2000;

    double rightAscension = degToRad(position.rightAscension * 15);
    double declination = degToRad(position.declination);

    double aberrationConstant =
        20.49552 * ARCSEC_TO_DEG;  // Constant of aberration

    double deltaRightAscension =
        -aberrationConstant *
        (std::cos(rightAscension) * std::cos(degToRad(meanLongitude)) *
             std::cos(degToRad(perihelionLongitude)) +
         std::sin(rightAscension) * std::sin(degToRad(meanLongitude))) /
        std::cos(declination);
    double deltaDeclination =
        -aberrationConstant *
        (std::sin(degToRad(perihelionLongitude)) *
         (std::sin(declination) * std::cos(degToRad(meanLongitude)) -
          std::cos(declination) * std::sin(rightAscension) *
              std::sin(degToRad(meanLongitude))));

    return {radToDeg(rightAscension + degToRad(deltaRightAscension)) / 15.0,
            radToDeg(declination + degToRad(deltaDeclination))};
}

auto applyPrecession(const EquatorialCoordinates& position,
                     double fromJulianDate,
                     double toJulianDate) -> EquatorialCoordinates {
    double centuriesSinceJ2000 = (fromJulianDate - JD2000) / CENTURY;
    double centuriesBetweenDates = (toJulianDate - fromJulianDate) / CENTURY;

    double zeta = (2306.2181 + 1.39656 * centuriesSinceJ2000 -
                   0.000139 * centuriesSinceJ2000 * centuriesSinceJ2000) *
                      centuriesBetweenDates +
                  (0.30188 - 0.000344 * centuriesSinceJ2000) *
                      centuriesBetweenDates * centuriesBetweenDates +
                  0.017998 * centuriesBetweenDates * centuriesBetweenDates *
                      centuriesBetweenDates;
    double z = (2306.2181 + 1.39656 * centuriesSinceJ2000 -
                0.000139 * centuriesSinceJ2000 * centuriesSinceJ2000) *
                   centuriesBetweenDates +
               (1.09468 + 0.000066 * centuriesSinceJ2000) *
                   centuriesBetweenDates * centuriesBetweenDates +
               0.018203 * centuriesBetweenDates * centuriesBetweenDates *
                   centuriesBetweenDates;
    double theta = (2004.3109 - 0.85330 * centuriesSinceJ2000 -
                    0.000217 * centuriesSinceJ2000 * centuriesSinceJ2000) *
                       centuriesBetweenDates -
                   (0.42665 + 0.000217 * centuriesSinceJ2000) *
                       centuriesBetweenDates * centuriesBetweenDates -
                   0.041833 * centuriesBetweenDates * centuriesBetweenDates *
                       centuriesBetweenDates;

    zeta = degToRad(zeta * ARCSEC_TO_DEG);
    z = degToRad(z * ARCSEC_TO_DEG);
    theta = degToRad(theta * ARCSEC_TO_DEG);

    double rightAscension = degToRad(position.rightAscension * 15);
    double declination = degToRad(position.declination);

    double A = std::cos(declination) * std::sin(rightAscension + zeta);
    double B = std::cos(theta) * std::cos(declination) *
                   std::cos(rightAscension + zeta) -
               std::sin(theta) * std::sin(declination);
    double C = std::sin(theta) * std::cos(declination) *
                   std::cos(rightAscension + zeta) +
               std::cos(theta) * std::sin(declination);

    double newRightAscension = std::atan2(A, B) + z;
    double newDeclination = std::asin(C);

    return {radToDeg(newRightAscension) / 15.0, radToDeg(newDeclination)};
}

auto observedToJ2000(const EquatorialCoordinates& observed,
                     double julianDate) -> EquatorialCoordinates {
    auto temp = applyAberration(observed, julianDate);
    temp = applyNutation(temp, julianDate, true);
    return applyPrecession(temp, julianDate, JD2000);
}

auto j2000ToObserved(const EquatorialCoordinates& j2000,
                     double julianDate) -> EquatorialCoordinates {
    auto temp = applyPrecession(j2000, JD2000, julianDate);
    temp = applyNutation(temp, julianDate);
    return applyAberration(temp, julianDate);
}

auto equatorialToHorizontal(const EquatorialCoordinates& object,
                            const GeographicCoordinates& observer,
                            double julianDate) -> HorizontalCoordinates {
    double localSiderealTime =
        range360(280.46061837 + 360.98564736629 * (julianDate - JD2000) +
                 observer.longitude);
    double hourAngle = range360(localSiderealTime - object.rightAscension * 15);

    double sinAltitude = std::sin(degToRad(object.declination)) *
                             std::sin(degToRad(observer.latitude)) +
                         std::cos(degToRad(object.declination)) *
                             std::cos(degToRad(observer.latitude)) *
                             std::cos(degToRad(hourAngle));
    double altitude = radToDeg(std::asin(sinAltitude));

    double cosAzimuth =
        (std::sin(degToRad(object.declination)) -
         std::sin(degToRad(altitude)) * std::sin(degToRad(observer.latitude))) /
        (std::cos(degToRad(altitude)) * std::cos(degToRad(observer.latitude)));
    double azimuth = radToDeg(std::acos(cosAzimuth));

    if (std::sin(degToRad(hourAngle)) > 0) {
        azimuth = 360 - azimuth;
    }

    return {range360(azimuth + 180), altitude};
}

auto horizontalToEquatorial(const HorizontalCoordinates& object,
                            const GeographicCoordinates& observer,
                            double julianDate) -> EquatorialCoordinates {
    double altitude = degToRad(object.altitude);
    double azimuth = degToRad(range360(object.azimuth + 180));
    double latitude = degToRad(observer.latitude);

    double sinDeclination =
        std::sin(altitude) * std::sin(latitude) +
        std::cos(altitude) * std::cos(latitude) * std::cos(azimuth);
    double declination = radToDeg(std::asin(sinDeclination));

    double cosHourAngle =
        (std::sin(altitude) -
         std::sin(latitude) * std::sin(degToRad(declination))) /
        (std::cos(latitude) * std::cos(degToRad(declination)));
    double hourAngle = radToDeg(std::acos(cosHourAngle));

    if (std::sin(azimuth) > 0) {
        hourAngle = 360 - hourAngle;
    }

    double localSiderealTime =
        range360(280.46061837 + 360.98564736629 * (julianDate - JD2000) +
                 observer.longitude);
    double rightAscension = range360(localSiderealTime - hourAngle) / 15.0;

    return {rightAscension, declination};
}

}  // namespace lithium
