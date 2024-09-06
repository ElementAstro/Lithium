#include "platesolver2.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "device/template/solver.hpp"

namespace fs = std::filesystem;

Platesolve2Solver::Platesolve2Solver(std::string executableLocation)
    : m_executableLocation(std::move(executableLocation)) , AtomSolver(executableLocation) {}

PlateSolveResult Platesolve2Solver::solve(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int imageWidth, int imageHeight) {
    int regions = 100;  // Default value, could be made configurable
    std::string outputFilePath = getOutputPath(imageFilePath);
    std::string arguments =
        getArguments(imageFilePath, initialCoordinates, fovW, fovH, regions);

    if (int result = executeCommand(m_executableLocation, arguments);
        result != 0) {
        std::cerr << "Error executing Platesolve2" << std::endl;
        return PlateSolveResult{false};
    }

    return readResult(outputFilePath, imageWidth, imageHeight);
}

std::string Platesolve2Solver::getOutputPath(
    const std::string& imageFilePath) const {
    fs::path path(imageFilePath);
    return (path.parent_path() / path.stem()).string() + ".apm";
}

std::string Platesolve2Solver::getArguments(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int regions) const {
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

PlateSolveResult Platesolve2Solver::readResult(
    const std::string& outputFilePath, int imageWidth, int imageHeight) const {
    PlateSolveResult result{false};
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
                result.coordinates.ra = std::stod(tokens[0]);
                result.coordinates.dec = std::stod(tokens[1]);
            }
        } else if (lineNum == 1 && tokens.size() > 2) {
            result.pixscale = std::stod(tokens[0]);
            result.positionAngle = 360 - std::stod(tokens[1]);
            result.flipped = (std::stod(tokens[2]) >= 0);
            if (*result.flipped) {
                result.positionAngle += 180;
            }
            if (!std::isnan(result.pixscale)) {
                double diagonalPixels = std::hypot(imageWidth, imageHeight);
                result.radius = (diagonalPixels * result.pixscale) /
                                (2 * 3600);  // Convert to degrees
            }
        }
        lineNum++;
    }

    return result;
}
