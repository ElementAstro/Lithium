#include "platesolver3.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

Platesolve3Solver::Platesolve3Solver(std::string executableLocation)
    : m_executableLocation(std::move(executableLocation)) {}

PlateSolveResult Platesolve3Solver::solve(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int imageWidth, int imageHeight) {
    std::string outputFilePath = getOutputPath(imageFilePath);
    std::string arguments =
        getArguments(imageFilePath, initialCoordinates, fovW, fovH);

    if (int result = executeCommand(m_executableLocation, arguments);
        result != 0) {
        std::cerr << "Error executing Platesolve3" << std::endl;
        return PlateSolveResult{false};
    }

    return readResult(outputFilePath, imageWidth, imageHeight);
}

std::string Platesolve3Solver::getOutputPath(
    const std::string& imageFilePath) const {
    fs::path path(imageFilePath);
    return (path.parent_path() / path.stem()).string() + "_PS3.txt";
}

std::string Platesolve3Solver::getArguments(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH) const {
    std::ostringstream oss;
    oss << "\"" << imageFilePath << "\" ";
    if (initialCoordinates) {
        oss << toRadians(initialCoordinates->ra) << " "
            << toRadians(initialCoordinates->dec) << " ";
    } else {
        oss << "0 0 ";
    }
    oss << toRadians(fovW) << " " << toRadians(fovH);
    return oss.str();
}

PlateSolveResult Platesolve3Solver::readResult(
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

        if (lineNum == 0) {
            result.success = (line == "True");
            if (!result.success)
                return result;
        } else if (lineNum == 1 && tokens.size() >= 2) {
            result.coordinates.ra = toDegrees(std::stod(tokens[0]));
            result.coordinates.dec = toDegrees(std::stod(tokens[1]));
        } else if (lineNum == 2 && tokens.size() >= 2) {
            result.pixscale = 206264.8 / std::stod(tokens[0]);
            if (!std::isnan(result.pixscale)) {
                result.radius =
                    arcsecToDegree(std::hypot(imageWidth * result.pixscale,
                                              imageHeight * result.pixscale) /
                                   2.0);
            }
            result.positionAngle = std::stod(tokens[1]);
        }
        lineNum++;
    }

    return result;
}
