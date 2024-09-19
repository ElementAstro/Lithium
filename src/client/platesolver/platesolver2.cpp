#include "platesolver2.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>

namespace fs = std::filesystem;

namespace {
    constexpr int DEFAULT_REGIONS = 100;
    constexpr double REVERSE_ANGLE_BASE = 360.0;
    constexpr double FLIP_ANGLE = 180.0;
    constexpr double SECONDS_IN_DEGREE = 3600.0;
}

Platesolve2Solver::Platesolve2Solver(std::string executableLocation)
    : AtomSolver(executableLocation), executableLocation_(std::move(executableLocation)) {}

auto Platesolve2Solver::solve(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int imageWidth, int imageHeight) -> PlateSolveResult {
    int regions = DEFAULT_REGIONS;  // Default value, could be made configurable
    std::string outputFilePath = getOutputPath(imageFilePath);
    std::string arguments =
        getArguments(imageFilePath, initialCoordinates, fovW, fovH, regions);

    if (int result = executeCommand(executableLocation_, arguments);
        result != 0) {
        std::cerr << "Error executing Platesolve2" << std::endl;
        return PlateSolveResult{false, {std::nullopt}, {std::nullopt}, {std::nullopt}, {std::nullopt}};
    }

    return readResult(outputFilePath, imageWidth, imageHeight);
}

auto Platesolve2Solver::getOutputPath(
    const std::string& imageFilePath) const -> std::string {
    fs::path path(imageFilePath);
    return (path.parent_path() / path.stem()).string() + ".apm";
}

auto Platesolve2Solver::getArguments(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int regions) const -> std::string {
    std::ostringstream oss;
    if (initialCoordinates) {
        oss << initialCoordinates->ra << "," << initialCoordinates->dec << ",";
    } else {
        oss << "0,0,";
    }
    oss << fovW << "," << fovH << "," << regions << "," << imageFilePath
        << ",0";
    return oss.str();
}

auto Platesolve2Solver::readResult(
    const std::string& outputFilePath, int imageWidth, int imageHeight) const -> PlateSolveResult {
    PlateSolveResult result{false, std::nullopt, std::nullopt, std::nullopt, std::nullopt};
    std::ifstream file(outputFilePath);
    if (!file.is_open()) {
        return result;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }

        if (lineNum == 0 && tokens.size() > 2) {
            result.success = (std::stoi(tokens[2]) == 1);
            if (result.success) {
                result.coordinates = Coordinates{std::stod(tokens[0]), std::stod(tokens[1])};
            }
        } else if (lineNum == 1 && tokens.size() > 2) {
            result.pixscale = std::stod(tokens[0]);
            result.positionAngle = REVERSE_ANGLE_BASE - std::stod(tokens[1]);
            result.flipped = (std::stod(tokens[2]) >= 0);
            if (*result.flipped) {
                result.positionAngle += FLIP_ANGLE;
            }
            if (!std::isnan(result.pixscale)) {
                double diagonalPixels = std::hypot(imageWidth, imageHeight);
                result.radius = (diagonalPixels * result.pixscale) /
                                (2 * SECONDS_IN_DEGREE);  // Convert to degrees
            }
        }
        lineNum++;
    }

    return result;
}