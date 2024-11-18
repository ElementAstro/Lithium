#include "croods.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <sstream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "atom/log/loguru.hpp"

namespace lithium::tools {

// 常量定义
constexpr double K_DEGREES_TO_RADIANS = M_PI / 180.0;
constexpr double K_RADIANS_TO_DEGREES = 180.0 / M_PI;
constexpr double K_HOURS_IN_DAY = 24.0;
constexpr double K_DEGREES_IN_CIRCLE = 360.0;
constexpr double K_MINUTES_IN_HOUR = 60.0;
constexpr double K_SECONDS_IN_MINUTE = 60.0;
constexpr double K_SECONDS_IN_HOUR = 3600.0;
constexpr double K_HOURS_TO_DEGREES = 15.0;
constexpr double K_EPSILON_VALUE = 1e-5;
constexpr double K_J2000_EPOCH = 2451545.0;
constexpr double K_JULIAN_CENTURY = 36525.0;
constexpr double K_SECONDS_IN_DAY = 86400.0;
constexpr double K_GST_COEFF1 = 280.46061837;
constexpr double K_GST_COEFF2 = 360.98564736629;
constexpr double K_GST_COEFF3 = 0.000387933;
constexpr double K_GST_COEFF4 = 38710000.0;

auto rangeTo(double value, double maxVal, double minVal) -> double {
    LOG_F(INFO, "rangeTo: value={:.6f}, max={:.6f}, min={:.6f}", value, maxVal,
          minVal);

    double period = maxVal - minVal;

    while (value < minVal) {
        value += period;
        LOG_F(INFO, "Adjusted value up: {:.6f}", value);
    }

    while (value > maxVal) {
        value -= period;
        LOG_F(INFO, "Adjusted value down: {:.6f}", value);
    }

    LOG_F(INFO, "Final value: {:.6f}", value);
    return value;
}

auto degreeToRad(double degree) -> double {
    double radians = degree * K_DEGREES_TO_RADIANS;
    LOG_F(INFO, "degreeToRad: {:.6f}° -> {:.6f} rad", degree, radians);
    return radians;
}

auto radToDegree(double radians) -> double {
    double degrees = radians * K_RADIANS_TO_DEGREES;
    LOG_F(INFO, "radToDegree: {:.6f} rad -> {:.6f}°", radians, degrees);
    return degrees;
}

auto hourToDegree(double hours) -> double {
    double degrees = hours * K_HOURS_TO_DEGREES;
    degrees = rangeTo(degrees, K_DEGREES_IN_CIRCLE, 0.0);
    LOG_F(INFO, "hourToDegree: {:.6f}h -> {:.6f}°", hours, degrees);
    return degrees;
}

auto hourToRad(double hours) -> double {
    double degrees = hours * K_HOURS_TO_DEGREES;
    degrees = rangeTo(degrees, K_DEGREES_IN_CIRCLE, 0.0);
    double radians = degreeToRad(degrees);
    LOG_F(INFO, "hourToRad: {:.6f}h -> {:.6f} rad", hours, radians);
    return radians;
}

auto degreeToHour(double degrees) -> double {
    double hours = degrees / K_HOURS_TO_DEGREES;
    hours = rangeTo(hours, K_HOURS_IN_DAY, 0.0);
    LOG_F(INFO, "degreeToHour: {:.6f}° -> {:.6f}h", degrees, hours);
    return hours;
}

auto radToHour(double radians) -> double {
    double degrees = radToDegree(radians);
    degrees = rangeTo(degrees, K_DEGREES_IN_CIRCLE, 0.0);
    double hours = degreeToHour(degrees);
    LOG_F(INFO, "radToHour: {:.6f} rad -> {:.6f}h", radians, hours);
    return hours;
}

auto getHaDegree(double rightAscensionRad, double lstDegree) -> double {
    double hourAngle = lstDegree - radToDegree(rightAscensionRad);
    hourAngle = rangeTo(hourAngle, K_DEGREES_IN_CIRCLE, 0.0);
    LOG_F(INFO, "getHaDegree: RA={:.6f} rad, LST={:.6f}° -> HA={:.6f}°",
          rightAscensionRad, lstDegree, hourAngle);
    return hourAngle;
}

auto raDecToAltAz(double hourAngleRad, double declinationRad,
                  double latitudeRad) -> std::vector<double> {
    LOG_F(INFO,
          "raDecToAltAz input: HA={:.6f} rad, Dec={:.6f} rad, Lat={:.6f} rad",
          hourAngleRad, declinationRad, latitudeRad);

    double cosLatitude = std::cos(latitudeRad);

    auto altitudeRad = std::asin(
        std::sin(latitudeRad) * std::sin(declinationRad) +
        cosLatitude * std::cos(declinationRad) * std::cos(hourAngleRad));

    double azimuthRad;
    if (cosLatitude < K_EPSILON_VALUE) {
        azimuthRad = hourAngleRad;  // polar case
        LOG_F(INFO, "Polar case detected, using HA as azimuth");
    } else {
        double temp =
            std::acos((std::sin(declinationRad) -
                       std::sin(altitudeRad) * std::sin(latitudeRad)) /
                      (std::cos(altitudeRad) * cosLatitude));

        azimuthRad = std::sin(hourAngleRad) > 0 ? 2 * M_PI - temp : temp;
        LOG_F(INFO, "Calculated azimuth temp={:.6f}, final={:.6f} rad", temp,
              azimuthRad);
    }

    LOG_F(INFO, "raDecToAltAz output: Alt={:.6f} rad, Az={:.6f} rad",
          altitudeRad, azimuthRad);
    return {altitudeRad, azimuthRad};
}

void altAzToRaDec(double altRadian, double azRadian, double& hrRadian,
                  double& decRadian, double latRadian) {
    LOG_F(INFO,
          "altAzToRaDec input: Alt={:.6f} rad, Az={:.6f} rad, Lat={:.6f} rad",
          altRadian, azRadian, latRadian);

    double cosLatitude = std::cos(latRadian);
    if (altRadian > M_PI / 2.0) {
        altRadian = M_PI - altRadian;
        azRadian += M_PI;
    }
    if (altRadian < -M_PI / 2.0) {
        altRadian = -M_PI - altRadian;
        azRadian -= M_PI;
    }
    double sinDec = std::sin(latRadian) * std::sin(altRadian) +
                    cosLatitude * std::cos(altRadian) * std::cos(azRadian);
    decRadian = std::asin(sinDec);
    if (cosLatitude < K_EPSILON_VALUE) {
        hrRadian = azRadian + M_PI;
    } else {
        double temp = cosLatitude * std::cos(decRadian);
        temp = (std::sin(altRadian) - std::sin(latRadian) * sinDec) / temp;
        temp = std::acos(std::clamp(-temp, -1.0, 1.0));
        if (std::sin(azRadian) > 0.0) {
            hrRadian = M_PI + temp;
        } else {
            hrRadian = M_PI - temp;
        }
    }
    LOG_F(INFO, "altAzToRaDec output: HR={:.6f} rad, Dec={:.6f} rad", hrRadian,
          decRadian);
}

auto periodBelongs(double value, double minVal, double maxVal, double period,
                   bool minInclusive, bool maxInclusive) -> bool {
    LOG_F(INFO,
          "periodBelongs: value={:.6f}, min={:.6f}, max={:.6f}, period={:.6f}, "
          "minInclusive={}, maxInclusive={}",
          value, minVal, maxVal, period, minInclusive, maxInclusive);

    int periodIndex = static_cast<int>((value - maxVal) / period);
    std::array<std::array<double, 2>, 3> ranges = {
        {{minVal + (periodIndex - 1) * period,
          maxVal + (periodIndex - 1) * period},
         {minVal + periodIndex * period, maxVal + periodIndex * period},
         {minVal + (periodIndex + 1) * period,
          maxVal + (periodIndex + 1) * period}}};

    for (const auto& range : ranges) {
        if ((maxInclusive && minInclusive &&
             (value >= range[0] && value <= range[1])) ||
            (maxInclusive && !minInclusive &&
             (value > range[0] && value <= range[1])) ||
            (!maxInclusive && !minInclusive &&
             (value > range[0] && value < range[1])) ||
            (!maxInclusive && minInclusive &&
             (value >= range[0] && value < range[1]))) {
            LOG_F(INFO, "Value belongs to range: [{:.6f}, {:.6f}]", range[0],
                  range[1]);
            return true;
        }
    }

    LOG_F(INFO, "Value does not belong to any range");
    return false;
}

auto convertEquatorialToCartesian(double rightAscension, double declination,
                                  double radius) -> CartesianCoordinates {
    LOG_F(
        INFO,
        "convertEquatorialToCartesian: RA={:.6f}°, Dec={:.6f}°, Radius={:.6f}",
        rightAscension, declination, radius);

    double raRad = degreeToRad(rightAscension);
    double decRad = degreeToRad(declination);

    double x = radius * std::cos(decRad) * std::cos(raRad);
    double y = radius * std::cos(decRad) * std::sin(raRad);
    double z = radius * std::sin(decRad);

    LOG_F(INFO, "Cartesian coordinates: x={:.6f}, y={:.6f}, z={:.6f}", x, y, z);
    return {x, y, z};
}

auto calculateVector(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& pointB)
    -> CartesianCoordinates {
    LOG_F(INFO,
          "calculateVector: PointA=({:.6f}, {:.6f}, {:.6f}), PointB=({:.6f}, "
          "{:.6f}, {:.6f})",
          pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z);

    CartesianCoordinates vector = {pointB.x - pointA.x, pointB.y - pointA.y,
                                   pointB.z - pointA.z};
    LOG_F(INFO, "Vector: x={:.6f}, y={:.6f}, z={:.6f}", vector.x, vector.y,
          vector.z);
    return vector;
}

auto calculatePointC(const CartesianCoordinates& pointA,
                     const CartesianCoordinates& vectorV)
    -> CartesianCoordinates {
    LOG_F(INFO,
          "calculatePointC: PointA=({:.6f}, {:.6f}, {:.6f}), Vector=({:.6f}, "
          "{:.6f}, {:.6f})",
          pointA.x, pointA.y, pointA.z, vectorV.x, vectorV.y, vectorV.z);

    CartesianCoordinates pointC = {pointA.x + vectorV.x, pointA.y + vectorV.y,
                                   pointA.z + vectorV.z};
    LOG_F(INFO, "PointC: x={:.6f}, y={:.6f}, z={:.6f}", pointC.x, pointC.y,
          pointC.z);
    return pointC;
}

auto convertToSphericalCoordinates(const CartesianCoordinates& cartesianPoint)
    -> std::optional<SphericalCoordinates> {
    LOG_F(INFO,
          "convertToSphericalCoordinates: Cartesian=({:.6f}, {:.6f}, {:.6f})",
          cartesianPoint.x, cartesianPoint.y, cartesianPoint.z);

    double x = cartesianPoint.x;
    double y = cartesianPoint.y;
    double z = cartesianPoint.z;

    double radius = std::sqrt(x * x + y * y + z * z);
    if (radius == 0.0) {
        LOG_F(WARNING, "Radius is zero, returning nullopt");
        return std::nullopt;
    }

    double declination = std::asin(z / radius) * K_RADIANS_TO_DEGREES;
    double rightAscension = std::atan2(y, x) * K_RADIANS_TO_DEGREES;

    if (rightAscension < 0) {
        rightAscension += K_DEGREES_IN_CIRCLE;
    }

    LOG_F(INFO, "Spherical coordinates: RA={:.6f}°, Dec={:.6f}°",
          rightAscension, declination);
    return SphericalCoordinates{rightAscension, declination};
}

auto calculateFOV(int focalLength, double cameraSizeWidth,
                  double cameraSizeHeight) -> MinMaxFOV {
    LOG_F(
        INFO,
        "calculateFOV: FocalLength={}, CameraWidth={:.6f}, CameraHeight={:.6f}",
        focalLength, cameraSizeWidth, cameraSizeHeight);

    double cameraSizeDiagonal = std::hypot(cameraSizeWidth, cameraSizeHeight);

    double minFOV = 2 * std::atan(cameraSizeHeight / (2.0 * focalLength)) *
                    K_RADIANS_TO_DEGREES;
    double maxFOV = 2 * std::atan(cameraSizeDiagonal / (2.0 * focalLength)) *
                    K_RADIANS_TO_DEGREES;

    LOG_F(INFO, "FOV: Min={:.6f}°, Max={:.6f}°", minFOV, maxFOV);
    return {minFOV, maxFOV};
}

auto calculateGST(const std::tm& date) -> double {
    LOG_F(INFO, "calculateGST: Date={:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
          date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour,
          date.tm_min, date.tm_sec);

    std::tm epoch = {0, 0, 12, 1, 0, 100, 0, 0, 0};  // Jan 1, 2000 12:00:00 UTC
    std::time_t epochTime = std::mktime(&epoch);
    std::time_t nowTime = std::mktime(const_cast<std::tm*>(&date));
    double julianDate =
        K_J2000_EPOCH + (nowTime - epochTime) / K_SECONDS_IN_DAY;
    double julianCenturies = (julianDate - K_J2000_EPOCH) / K_JULIAN_CENTURY;
    double gst =
        K_GST_COEFF1 + K_GST_COEFF2 * (julianDate - K_J2000_EPOCH) +
        K_GST_COEFF3 * julianCenturies * julianCenturies -
        (julianCenturies * julianCenturies * julianCenturies / K_GST_COEFF4);
    gst = std::fmod(gst, K_DEGREES_IN_CIRCLE);

    LOG_F(INFO, "GST: {:.6f}°", gst);
    return gst;
}

auto calculateAltAz(double rightAscension, double declination, double latitude,
                    double longitude, const std::tm& date) -> AltAz {
    LOG_F(INFO,
          "calculateAltAz: RA={:.6f}°, Dec={:.6f}°, Lat={:.6f}°, Lon={:.6f}°, "
          "Date={:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
          rightAscension, declination, latitude, longitude, date.tm_year + 1900,
          date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min,
          date.tm_sec);

    // 将输入值转换为弧度
    double raRad = degreeToRad(rightAscension *
                               K_HOURS_TO_DEGREES);  // 赤经转换为弧度并乘以15
    double decRad = degreeToRad(declination);
    double latRad = degreeToRad(latitude);
    double lonRad = degreeToRad(longitude);

    // 计算GST和LST
    double gst = calculateGST(date);
    double lst =
        std::fmod(gst + longitude, K_DEGREES_IN_CIRCLE);  // LST在0-360度范围内
    double hourAngle = degreeToRad(lst) - raRad;          // 时角

    // 计算高度角
    double altRad =
        std::asin(std::sin(decRad) * std::sin(latRad) +
                  std::cos(decRad) * std::cos(latRad) * std::cos(hourAngle));
    double altDeg = radToDegree(altRad);

    // 计算方位角
    double cosAz = (std::sin(decRad) - std::sin(altRad) * std::sin(latRad)) /
                   (std::cos(altRad) * std::cos(latRad));
    double azRad = std::acos(cosAz);
    double azDeg = radToDegree(azRad);

    // 调整方位角
    if (std::sin(hourAngle) > 0) {
        azDeg = K_DEGREES_IN_CIRCLE - azDeg;
    }

    LOG_F(INFO, "AltAz: Alt={:.6f}°, Az={:.6f}°", altDeg, azDeg);
    return {altDeg, azDeg};
}

void printDMS(double angle) {
    int degrees = static_cast<int>(angle);
    double fractional = angle - degrees;
    int minutes = static_cast<int>(fractional * K_MINUTES_IN_HOUR);
    double seconds =
        (fractional * K_MINUTES_IN_HOUR - minutes) * K_SECONDS_IN_MINUTE;

    LOG_F(INFO, "{}° {}' {:.2f}\"", degrees, minutes, seconds);
}

auto dmsToDegree(int degrees, int minutes, double seconds) -> double {
    LOG_F(INFO, "dmsToDegree: Degrees={}, Minutes={}, Seconds={:.6f}", degrees,
          minutes, seconds);

    // 确定符号
    double sign = degrees < 0 ? -1.0 : 1.0;
    // 计算绝对值
    double absDegrees = std::abs(degrees) + minutes / K_MINUTES_IN_HOUR +
                        seconds / K_SECONDS_IN_HOUR;
    double result = sign * absDegrees;

    LOG_F(INFO, "Result: {:.6f}°", result);
    return result;
}

auto radToDmsStr(double radians) -> std::string {
    LOG_F(INFO, "radToDmsStr: Input radians={:.6f}", radians);

    // 将弧度转换为度数
    double degrees = radToDegree(radians);

    // 确定符号
    char sign = degrees < 0 ? '-' : '+';
    degrees = std::abs(degrees);

    // 提取度分秒
    int deg = static_cast<int>(degrees);
    double minPartial = (degrees - deg) * 60.0;
    int min = static_cast<int>(minPartial);
    double sec = (minPartial - min) * 60.0;

    // 处理舍入误差
    if (sec >= 60.0) {
        sec = 0.0;
        min++;
        if (min >= 60) {
            min = 0;
            deg++;
        }
    }

    // 格式化输出
    std::stringstream ss;
    ss << sign << std::setfill('0') << std::setw(2) << deg << "°"
       << std::setfill('0') << std::setw(2) << min << "'" << std::fixed
       << std::setprecision(1) << sec << "\"";

    std::string result = ss.str();
    LOG_F(INFO, "radToDmsStr: Output={}", result);
    return result;
}

auto radToHmsStr(double radians) -> std::string {
    LOG_F(INFO, "radToHmsStr: Input radians={:.6f}", radians);

    // 将弧度转换为小时
    double hours = radToHour(radians);

    // 确保hours在0-24范围内
    hours = rangeTo(hours, 24.0, 0.0);

    // 提取时分秒
    int hrs = static_cast<int>(hours);
    double minPartial = (hours - hrs) * 60.0;
    int min = static_cast<int>(minPartial);
    double sec = (minPartial - min) * 60.0;

    // 处理舍入误差
    if (sec >= 60.0) {
        sec = 0.0;
        min++;
        if (min >= 60) {
            min = 0;
            hrs++;
            if (hrs >= 24) {
                hrs = 0;
            }
        }
    }

    // 格式化输出
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hrs << ':' << std::setfill('0')
       << std::setw(2) << min << ':' << std::fixed << std::setprecision(1)
       << std::setfill('0') << std::setw(4) << sec;

    std::string result = ss.str();
    LOG_F(INFO, "radToHmsStr: Output={}", result);
    return result;
}

inline std::string formatTime(const std::chrono::system_clock::time_point& time,
                              bool isLocal,
                              const std::string& format = "%H:%M:%S") {
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = isLocal ? *std::localtime(&tt) : *std::gmtime(&tt);
    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str() + (isLocal ? "(Local)" : "(UTC)");
}

auto getInfoTextA(const std::chrono::system_clock::time_point& localTime,
                  double raDegree, double decDegree, double dRaDegree,
                  double dDecDegree, const std::string& mountStatus,
                  const std::string& guideStatus) -> std::string {
    std::vector<size_t> start = {0, 16, 23, 50, 65, 75, 90, 103};
    std::vector<std::string> strs(8);

    strs[0] = formatTime(localTime, true);
    strs[1] = "RA/DEC";
    strs[2] = radToHmsStr(degreeToRad(raDegree)) + " " +
              radToDmsStr(degreeToRad(decDegree));
    strs[3] = mountStatus;
    strs[4] = guideStatus;
    strs[5] =
        "RMS " + std::to_string(dRaDegree) + "/" + std::to_string(dDecDegree);

    std::string result(120, ' ');  // Pre-allocate with spaces
    for (size_t i = 0; i < start.size(); ++i) {
        result.replace(start[i], strs[i].length(), strs[i]);
    }
    return result;
}

auto getInfoTextB(const std::chrono::system_clock::time_point& utcTime,
                  double azRad, double altRad, const std::string& camStatus,
                  double camTemp, double camTargetTemp, int camX, int camY,
                  int cfwPos, const std::string& cfwName,
                  const std::string& cfwStatus) -> std::string {
    std::vector<size_t> start = {0, 16, 24, 50, 65, 75, 90, 103};
    std::vector<std::string> strs(8);

    strs[0] = formatTime(utcTime, false);
    strs[1] = "AZ/ALT";
    strs[2] = radToDmsStr(azRad) + " " + radToDmsStr(altRad);
    strs[3] = camStatus;
    strs[4] = std::to_string(camTemp) + "/" + std::to_string(camTargetTemp);
    strs[5] = std::to_string(camX) + "*" + std::to_string(camY);
    strs[6] = "CFW " + cfwStatus;
    strs[7] = "#" + std::to_string(cfwPos) + " " + cfwName;

    std::string result(120, ' ');
    for (size_t i = 0; i < start.size(); ++i) {
        result.replace(start[i], strs[i].length(), strs[i]);
    }
    return result;
}

auto getInfoTextC(int cpuTemp, int cpuLoad, double diskFree,
                  double longitudeRad, double latitudeRad, double raJ2000,
                  double decJ2000, double az, double alt,
                  const std::string& objName) -> std::string {
    std::vector<size_t> start = {0, 16, 23, 50, 65, 120, 121, 122};
    std::vector<std::string> strs(8);

    strs[0] =
        "CPU " + std::to_string(cpuTemp) + "C " + std::to_string(cpuLoad) + "%";
    strs[1] = "Site";
    strs[2] = radToDmsStr(longitudeRad) + " " + radToDmsStr(latitudeRad);
    strs[3] = "Free " + std::to_string(diskFree) + "G";
    strs[4] = "Info: " + objName + radToHmsStr(raJ2000) + " " +
              radToDmsStr(decJ2000) + " " + radToDmsStr(M_PI - az) + " " +
              radToDmsStr(alt);

    std::string result(150, ' ');
    for (size_t i = 0; i < start.size(); ++i) {
        result.replace(start[i], strs[i].length(), strs[i]);
    }
    return result;
}
}  // namespace lithium::tools