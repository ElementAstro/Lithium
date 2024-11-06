#include <pybind11/chrono.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "tools/croods.hpp"
#include "tools/libastro.hpp"

namespace py = pybind11;
using namespace lithium::tools;

PYBIND11_MODULE(croods, m) {
    m.doc() = "Croods Module";

    py::class_<CartesianCoordinates>(m, "CartesianCoordinates")
        .def(py::init<>())
        .def_readwrite("x", &CartesianCoordinates::x)
        .def_readwrite("y", &CartesianCoordinates::y)
        .def_readwrite("z", &CartesianCoordinates::z);

    py::class_<SphericalCoordinates>(m, "SphericalCoordinates")
        .def(py::init<>())
        .def_readwrite("rightAscension", &SphericalCoordinates::rightAscension)
        .def_readwrite("declination", &SphericalCoordinates::declination);

    py::class_<MinMaxFOV>(m, "MinMaxFOV")
        .def(py::init<>())
        .def_readwrite("minFOV", &MinMaxFOV::minFOV)
        .def_readwrite("maxFOV", &MinMaxFOV::maxFOV);

    py::class_<DateTime>(m, "DateTime")
        .def(py::init<>())
        .def_readwrite("year", &DateTime::year)
        .def_readwrite("month", &DateTime::month)
        .def_readwrite("day", &DateTime::day)
        .def_readwrite("hour", &DateTime::hour)
        .def_readwrite("minute", &DateTime::minute)
        .def_readwrite("second", &DateTime::second);

    py::class_<CelestialCoords<double>>(m, "CelestialCoords")
        .def(py::init<>())
        .def_readwrite("ra", &CelestialCoords<double>::ra)
        .def_readwrite("dec", &CelestialCoords<double>::dec);

    py::class_<GeographicCoords<double>>(m, "GeographicCoords")
        .def(py::init<>())
        .def_readwrite("latitude", &GeographicCoords<double>::latitude)
        .def_readwrite("longitude", &GeographicCoords<double>::longitude);

    m.def("range_to", &rangeTo, py::arg("value"), py::arg("max"),
          py::arg("min"), "Clamps a value to a specified range.");
    m.def("degree_to_rad", &degreeToRad, py::arg("degree"),
          "Converts degrees to radians.");
    m.def("rad_to_degree", &radToDegree, py::arg("rad"),
          "Converts radians to degrees.");
    m.def("hour_to_degree", &hourToDegree, py::arg("hour"),
          "Converts hours to degrees.");
    m.def("hour_to_rad", &hourToRad, py::arg("hour"),
          "Converts hours to radians.");
    m.def("degree_to_hour", &degreeToHour, py::arg("degree"),
          "Converts degrees to hours.");
    m.def("rad_to_hour", &radToHour, py::arg("rad"),
          "Converts radians to hours.");
    m.def("get_ha_degree", &getHaDegree, py::arg("RA_radian"),
          py::arg("LST_Degree"), "Calculates the hour angle in degrees.");
    m.def("ra_dec_to_alt_az",
          py::overload_cast<double, double, double&, double&, double>(
              &raDecToAltAz),
          py::arg("ha_radian"), py::arg("dec_radian"), py::arg("alt_radian"),
          py::arg("az_radian"), py::arg("lat_radian"),
          "Converts RA/Dec to Alt/Az.");
    m.def("ra_dec_to_alt_az",
          py::overload_cast<double, double, double>(&raDecToAltAz),
          py::arg("ha_radian"), py::arg("dec_radian"), py::arg("lat_radian"),
          "Converts RA/Dec to Alt/Az and returns a vector.");
    m.def("period_belongs", &periodBelongs, py::arg("value"), py::arg("min"),
          py::arg("max"), py::arg("period"), py::arg("minequ"),
          py::arg("maxequ"),
          "Checks if a value belongs to a specified period.");
    m.def("convert_equatorial_to_cartesian", &convertEquatorialToCartesian,
          py::arg("ra"), py::arg("dec"), py::arg("radius"),
          "Converts equatorial coordinates to Cartesian coordinates.");
    m.def("calculate_vector", &calculateVector, py::arg("pointA"),
          py::arg("pointB"),
          "Calculates the vector between two Cartesian points.");
    m.def("calculate_point_c", &calculatePointC, py::arg("pointA"),
          py::arg("vectorV"),
          "Calculates a new Cartesian point based on a vector.");
    m.def("convert_to_spherical_coordinates", &convertToSphericalCoordinates,
          py::arg("cartesianPoint"),
          "Converts Cartesian coordinates to spherical coordinates.");
    m.def("calculate_fov", &calculateFOV, py::arg("focalLength"),
          py::arg("cameraSizeWidth"), py::arg("cameraSizeHeight"),
          "Calculates the field of view based on camera parameters.");
    m.def("lumen", &lumen, py::arg("wavelength"),
          "Calculates the luminous efficacy for a given wavelength.");
    m.def("redshift", &redshift, py::arg("observed"), py::arg("rest"),
          "Calculates the redshift.");
    m.def("doppler", &doppler, py::arg("redshift"), py::arg("speed"),
          "Calculates the Doppler shift.");
    m.def("range_ha", &rangeHA<double>, py::arg("range"),
          "Clamps a value to the range of hour angles.");
    m.def("range_24", &range24<double>, py::arg("range"),
          "Clamps a value to the range of 24 hours.");
    m.def("range_360", &range360<double>, py::arg("range"),
          "Clamps a value to the range of 360 degrees.");
    m.def("range_dec", &rangeDec<double>, py::arg("decDegrees"),
          "Clamps a declination value to the valid range.");
    m.def("get_local_hour_angle", &getLocalHourAngle<double>,
          py::arg("siderealTime"), py::arg("rightAscension"),
          "Calculates the local hour angle.");
    m.def("get_alt_az_coordinates", &getAltAzCoordinates<double>,
          py::arg("hourAngle"), py::arg("declination"), py::arg("latitude"),
          "Calculates the altitude and azimuth coordinates.");
    m.def("estimate_geocentric_elevation", &estimateGeocentricElevation<double>,
          py::arg("latitude"), py::arg("elevation"),
          "Estimates the geocentric elevation.");
    m.def("estimate_field_rotation_rate", &estimateFieldRotationRate<double>,
          py::arg("altitude"), py::arg("azimuth"), py::arg("latitude"),
          "Estimates the field rotation rate.");
    m.def("estimate_field_rotation", &estimateFieldRotation<double>,
          py::arg("hourAngle"), py::arg("rate"),
          "Estimates the field rotation.");
    m.def("as2rad", &as2rad, py::arg("arcSeconds"),
          "Converts arcseconds to radians.");
    m.def("rad2as", &rad2as, py::arg("radians"),
          "Converts radians to arcseconds.");
    m.def("estimate_distance", &estimateDistance<double>, py::arg("parsecs"),
          py::arg("parallaxRadius"),
          "Estimates the distance based on parallax.");
    m.def("m2au", &m2au, py::arg("meters"),
          "Converts meters to astronomical units.");
    m.def("calc_delta_magnitude", &calcDeltaMagnitude<double>,
          py::arg("magnitudeRatio"), py::arg("spectrum"),
          "Calculates the delta magnitude.");
    m.def("calc_star_mass", &calcStarMass<double>, py::arg("deltaMagnitude"),
          py::arg("referenceSize"), "Calculates the mass of a star.");
    m.def("estimate_orbit_radius", &estimateOrbitRadius<double>,
          py::arg("observedWavelength"), py::arg("referenceWavelength"),
          py::arg("period"), "Estimates the orbit radius.");
    m.def("estimate_secondary_mass", &estimateSecondaryMass<double>,
          py::arg("starMass"), py::arg("starDrift"), py::arg("orbitRadius"),
          "Estimates the mass of a secondary object.");
    m.def("estimate_secondary_size", &estimateSecondarySize<double>,
          py::arg("starSize"), py::arg("dropoffRatio"),
          "Estimates the size of a secondary object.");
    m.def("calc_photon_flux", &calcPhotonFlux<double>,
          py::arg("relativeMagnitude"), py::arg("filterBandwidth"),
          py::arg("wavelength"), py::arg("steradian"),
          "Calculates the photon flux.");
    m.def("calc_rel_magnitude", &calcRelMagnitude<double>,
          py::arg("photonFlux"), py::arg("filterBandwidth"),
          py::arg("wavelength"), py::arg("steradian"),
          "Calculates the relative magnitude.");
    m.def("estimate_absolute_magnitude", &estimateAbsoluteMagnitude<double>,
          py::arg("deltaDistance"), py::arg("deltaMagnitude"),
          "Estimates the absolute magnitude.");
    m.def("baseline_2d_projection", &baseline2dProjection<double>,
          py::arg("altitude"), py::arg("azimuth"),
          "Calculates the 2D projection of a baseline.");
    m.def("baseline_delay", &baselineDelay<double>, py::arg("altitude"),
          py::arg("azimuth"), py::arg("baseline"),
          "Calculates the baseline delay.");
    m.def("calculate_julian_date", &calculateJulianDate<double>,
          py::arg("dateTime"), "Calculates the Julian date.");
    m.def("calculate_sidereal_time", &calculateSiderealTime<double>,
          py::arg("dateTime"), py::arg("longitude"),
          "Calculates the sidereal time.");
    m.def("calculate_refraction", &calculateRefraction<double>,
          py::arg("altitude"), py::arg("temperature") = 10.0,
          py::arg("pressure") = 1010.0, "Calculates atmospheric refraction.");
    m.def("apply_parallax", &applyParallax<double>, py::arg("coords"),
          py::arg("observer"), py::arg("distance"), py::arg("dt"),
          "Applies parallax correction to celestial coordinates.");
    m.def("equatorial_to_ecliptic", &equatorialToEcliptic<double>,
          py::arg("coords"), py::arg("obliquity"),
          "Converts equatorial coordinates to ecliptic coordinates.");
    m.def("calculate_precession", &calculatePrecession<double>,
          py::arg("coords"), py::arg("from"), py::arg("to"),
          "Calculates the precession of celestial coordinates.");
    m.def("format_ra", &formatRa<double>, py::arg("ra"),
          "Formats right ascension as a string.");
    m.def("format_dec", &formatDec<double>, py::arg("dec"),
          "Formats declination as a string.");

    py::class_<EquatorialCoordinates>(m, "EquatorialCoordinates")
        .def(py::init<>())
        .def_readwrite("rightAscension", &EquatorialCoordinates::rightAscension)
        .def_readwrite("declination", &EquatorialCoordinates::declination);

    py::class_<HorizontalCoordinates>(m, "HorizontalCoordinates")
        .def(py::init<>())
        .def_readwrite("azimuth", &HorizontalCoordinates::azimuth)
        .def_readwrite("altitude", &HorizontalCoordinates::altitude);

    py::class_<GeographicCoordinates>(m, "GeographicCoordinates")
        .def(py::init<>())
        .def_readwrite("longitude", &GeographicCoordinates::longitude)
        .def_readwrite("latitude", &GeographicCoordinates::latitude)
        .def_readwrite("elevation", &GeographicCoordinates::elevation);

    m.def("deg_to_rad", &degToRad, py::arg("deg"),
          "Converts degrees to radians.");
    m.def("rad_to_deg", &radToDeg, py::arg("rad"),
          "Converts radians to degrees.");
    m.def("range_360", &range360<double>, py::arg("angle"),
          "Clamps an angle to the range 0 to 360 degrees.");

    m.def("observed_to_j2000", &observedToJ2000, py::arg("observed"),
          py::arg("julianDate"),
          "Converts observed equatorial coordinates to J2000 coordinates.");
    m.def("j2000_to_observed", &j2000ToObserved, py::arg("j2000"),
          py::arg("julianDate"),
          "Converts J2000 equatorial coordinates to observed coordinates.");
    m.def("equatorial_to_horizontal", &equatorialToHorizontal,
          py::arg("object"), py::arg("observer"), py::arg("julianDate"),
          "Converts equatorial coordinates to horizontal coordinates.");
    m.def("horizontal_to_equatorial", &horizontalToEquatorial,
          py::arg("object"), py::arg("observer"), py::arg("julianDate"),
          "Converts horizontal coordinates to equatorial coordinates.");

    m.def("get_nutation", &getNutation, py::arg("julianDate"),
          "Calculates the nutation for a given Julian date.");
    m.def("apply_nutation", &applyNutation, py::arg("position"),
          py::arg("julianDate"), py::arg("reverse") = false,
          "Applies nutation to equatorial coordinates.");
    m.def("apply_aberration", &applyAberration, py::arg("position"),
          py::arg("julianDate"),
          "Applies aberration to equatorial coordinates.");
    m.def("apply_precession", &applyPrecession, py::arg("position"),
          py::arg("fromJulianDate"), py::arg("toJulianDate"),
          "Applies precession to equatorial coordinates.");
}
