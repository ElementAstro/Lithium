#include "solverutils.hpp"

#include <cmath>

#include "atom/log/loguru.hpp"

namespace lithium::tools {
auto extractWCSParams(const std::string& wcsInfo) -> WCSParams {
    WCSParams wcs;

    size_t pos1 = wcsInfo.find("crpix0");
    size_t pos2 = wcsInfo.find("crpix1");
    size_t pos3 = wcsInfo.find("crval0");
    size_t pos4 = wcsInfo.find("crval1");
    size_t pos5 = wcsInfo.find("cd11");
    size_t pos6 = wcsInfo.find("cd12");
    size_t pos7 = wcsInfo.find("cd21");
    size_t pos8 = wcsInfo.find("cd22");

    auto extractValue = [&wcsInfo](size_t start, size_t offset) {
        size_t endPos = wcsInfo.find("\n", start);
        return std::stod(
            wcsInfo.substr(start + offset, endPos - start - offset));
    };

    wcs.crpix0 = extractValue(pos1, 7);
    wcs.crpix1 = extractValue(pos2, 7);
    wcs.crval0 = extractValue(pos3, 7);
    wcs.crval1 = extractValue(pos4, 7);
    wcs.cd11 = extractValue(pos5, 5);
    wcs.cd12 = extractValue(pos6, 5);
    wcs.cd21 = extractValue(pos7, 5);
    wcs.cd22 = extractValue(pos8, 5);

    /*
       // 使用 std::cout 输出调试信息，设置精度为9位
       std::cout << std::setprecision(9) << std::fixed;
       std::cout << "crpix0: " << wcs.crpix0 << std::endl;
       std::cout << "crpix1: " << wcs.crpix1 << std::endl;
       std::cout << "crval0: " << wcs.crval0 << std::endl;
       std::cout << "crval1: " << wcs.crval1 << std::endl;
       std::cout << "cd11: " << wcs.cd11 << std::endl;
       std::cout << "cd12: " << wcs.cd12 << std::endl;
       std::cout << "cd21: " << wcs.cd21 << std::endl;
       std::cout << "cd22: " << wcs.cd22 << std::endl;
    */

    return wcs;
}

auto pixelToRaDec(double x, double y,
                  const WCSParams& wcs) -> SphericalCoordinates {
    double dx = x - wcs.crpix0;
    double dy = y - wcs.crpix1;

    double ra = wcs.crval0 + wcs.cd11 * dx + wcs.cd12 * dy;
    double dec = wcs.crval1 + wcs.cd21 * dx + wcs.cd22 * dy;

    return {ra, dec};
}

auto getFOVCorners(const WCSParams& wcs, int imageWidth,
                   int imageHeight) -> std::vector<SphericalCoordinates> {
    std::vector<SphericalCoordinates> corners(4);
    corners[0] = pixelToRaDec(0, 0, wcs);                     // Bottom-left
    corners[1] = pixelToRaDec(imageWidth, 0, wcs);            // Bottom-right
    corners[2] = pixelToRaDec(imageWidth, imageHeight, wcs);  // Top-right
    corners[3] = pixelToRaDec(0, imageHeight, wcs);           // Top-left

    return corners;
}

auto calculateFOV(int focalLength, double cameraSizeWidth,
                  double cameraSizeHeight) -> MinMaxFOV {
    MinMaxFOV result;
    LOG_F(INFO, "Calculating FOV...");
    LOG_F(INFO, "FocalLength: {}, CameraSize: {}, {}", focalLength,
          cameraSizeWidth, cameraSizeHeight);

    double cameraSizeDiagonal =
        std::sqrt(std::pow(cameraSizeWidth, 2) + std::pow(cameraSizeHeight, 2));

    double minFOV =
        2 * std::atan(cameraSizeHeight / (2 * focalLength)) * 180 / M_PI;
    double maxFOV =
        2 * std::atan(cameraSizeDiagonal / (2 * focalLength)) * 180 / M_PI;

    result.minFOV = minFOV;
    result.maxFOV = maxFOV;

    LOG_F(INFO, "minFov: {}, maxFov: {}", result.minFOV, result.maxFOV);

    return result;
}
}  // namespace lithium::tools
