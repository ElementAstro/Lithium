#include "platesolver2.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace fs = std::filesystem;

namespace {
constexpr int DEFAULT_REGIONS = 100;
constexpr double REVERSE_ANGLE_BASE = 360.0;
constexpr double FLIP_ANGLE = 180.0;
constexpr double SECONDS_IN_DEGREE = 3600.0;
}  // namespace

class Platesolve2Solver::Impl {
public:
    Impl(std::string executableLocation)
        : executableLocation_(std::move(executableLocation)) {}

    auto solve(const std::string& imageFilePath,
               const std::optional<Coordinates>& initialCoordinates,
               double fovW, double fovH, int imageWidth,
               int imageHeight) -> PlateSolveResult {
        int regions =
            DEFAULT_REGIONS;  // Default value, could be made configurable
        std::string outputFilePath = getOutputPath(imageFilePath);
        std::string arguments = getArguments(imageFilePath, initialCoordinates,
                                             fovW, fovH, regions);

        std::string command = executableLocation_ + " " + arguments;
        if (int result = atom::system::executeCommandWithStatus(command).second;
            result != 0) {
            LOG_F(ERROR, "Error executing Platesolve2");
            return PlateSolveResult{false, {0, 0}, 0.0, 0};
        }

        return readResult(outputFilePath, imageWidth, imageHeight);
    }

    auto getOutputPath(const std::string& imageFilePath) const -> std::string {
        fs::path path(imageFilePath);
        return (path.parent_path() / path.stem()).string() + ".apm";
    }

    auto getArguments(const std::string& imageFilePath,
                      const std::optional<Coordinates>& initialCoordinates,
                      double fovW, double fovH,
                      int regions) const -> std::string {
        std::ostringstream oss;
        if (initialCoordinates) {
            oss << initialCoordinates->ra << "," << initialCoordinates->dec
                << ",";
        } else {
            oss << "0,0,";
        }
        oss << fovW << "," << fovH << "," << regions << "," << imageFilePath
            << ",0";
        return oss.str();
    }

    auto readResult(const std::string& outputFilePath, int imageWidth,
                    int imageHeight) const -> PlateSolveResult {
        PlateSolveResult result{false, {0, 0}, 0, 0};
        std::ifstream file(outputFilePath);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open result file: {}", outputFilePath);
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
                    result.coordinates =
                        Coordinates{std::stod(tokens[0]), std::stod(tokens[1])};
                }
            } else if (lineNum == 1 && tokens.size() > 2) {
                result.pixscale = std::stod(tokens[0]);
                result.positionAngle =
                    REVERSE_ANGLE_BASE - std::stod(tokens[1]);
                result.flipped = (std::stod(tokens[2]) >= 0);
                if (*result.flipped) {
                    result.positionAngle += FLIP_ANGLE;
                }
                if (!std::isnan(result.pixscale)) {
                    double diagonalPixels = std::hypot(imageWidth, imageHeight);
                    result.radius =
                        (diagonalPixels * result.pixscale) /
                        (2 * SECONDS_IN_DEGREE);  // Convert to degrees
                }
            }
            lineNum++;
        }

        return result;
    }

private:
    std::string executableLocation_;
};

Platesolve2Solver::Platesolve2Solver(std::string executableLocation)
    : AtomSolver(executableLocation),
      impl_(std::make_unique<Impl>(std::move(executableLocation))) {}

auto Platesolve2Solver::solve(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int imageWidth, int imageHeight) -> PlateSolveResult {
    return impl_->solve(imageFilePath, initialCoordinates, fovW, fovH,
                        imageWidth, imageHeight);
}

auto Platesolve2Solver::getOutputPath(const std::string& imageFilePath) const
    -> std::string {
    return impl_->getOutputPath(imageFilePath);
}