#pragma once

#include "device/template/solver.hpp"

class Platesolve2Solver : public AtomSolver {
public:
    explicit Platesolve2Solver(std::string executableLocation);

    auto solve(const std::string& imageFilePath,
               const std::optional<Coordinates>& initialCoordinates,
               double fovW, double fovH, int imageWidth,
               int imageHeight) -> PlateSolveResult override;

protected:
    [[nodiscard]] auto getOutputPath(const std::string& imageFilePath) const
        -> std::string override;

private:
    std::string executableLocation_;

    [[nodiscard]] auto getArguments(
        const std::string& imageFilePath,
        const std::optional<Coordinates>& initialCoordinates, double fovW,
        double fovH, int regions) const -> std::string;

    [[nodiscard]] auto readResult(const std::string& outputFilePath,
                                  int imageWidth,
                                  int imageHeight) const -> PlateSolveResult;
};