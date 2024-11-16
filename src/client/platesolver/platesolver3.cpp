#include "platesolver3.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

class Platesolve3Solver::Impl {
public:
    Impl(std::string executableLocation)
        : executableLocation_(std::move(executableLocation)) {}

    PlateSolveResult solve(const std::string& imageFilePath,
                           const std::optional<Coordinates>& initialCoordinates,
                           double fovW, double fovH, int imageWidth,
                           int imageHeight) {
        std::string outputFilePath = getOutputPath(imageFilePath);
        std::string arguments =
            getArguments(imageFilePath, initialCoordinates, fovW, fovH);

        if (int result = executeCommand(executableLocation_, arguments);
            result != 0) {
            LOG_F(ERROR, "Error executing Platesolve3");
            return PlateSolveResult{false};
        }

        return readResult(outputFilePath, imageWidth, imageHeight);
    }

    std::string getOutputPath(const std::string& imageFilePath) const {
        fs::path path(imageFilePath);
        return (path.parent_path() / path.stem()).string() + "_PS3.txt";
    }

private:
    std::string executableLocation_;

    std::string getArguments(
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

    PlateSolveResult readResult(const std::string& outputFilePath,
                                int imageWidth, int imageHeight) const {
        PlateSolveResult result{false};
        std::ifstream file(outputFilePath);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open result file: %s",
                  outputFilePath.c_str());
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
                    result.radius = arcsecToDegree(
                        std::hypot(imageWidth * result.pixscale,
                                   imageHeight * result.pixscale) /
                        2.0);
                }
                result.positionAngle = std::stod(tokens[1]);
            }
            lineNum++;
        }

        return result;
    }

    int executeCommand(const std::string& executable,
                       const std::string& args) const {
        std::string command = executable + " " + args;
        return std::system(command.c_str());
    }

    double toRadians(double degrees) const { return degrees * M_PI / 180.0; }

    double toDegrees(double radians) const { return radians * 180.0 / M_PI; }

    double arcsecToDegree(double arcsec) const { return arcsec / 3600.0; }
};

Platesolve3Solver::Platesolve3Solver(std::string executableLocation)
    : AtomSolver(std::move(executableLocation)),
      impl_(std::make_unique<Impl>(std::move(executableLocation))) {}

Platesolve3Solver::~Platesolve3Solver() = default;

PlateSolveResult Platesolve3Solver::solve(
    const std::string& imageFilePath,
    const std::optional<Coordinates>& initialCoordinates, double fovW,
    double fovH, int imageWidth, int imageHeight) {
    return impl_->solve(imageFilePath, initialCoordinates, fovW, fovH,
                        imageWidth, imageHeight);
}

std::string Platesolve3Solver::getOutputPath(
    const std::string& imageFilePath) const {
    return impl_->getOutputPath(imageFilePath);
}