#pragma once

#include "device/template/solver.hpp"

class Platesolve2Solver : public AtomSolver {
public:
    Platesolve2Solver(std::string executableLocation);

    PlateSolveResult solve(const std::string& imageFilePath,
                           const std::optional<Coordinates>& initialCoordinates,
                           double fovW, double fovH, int imageWidth,
                           int imageHeight) override;

protected:
    std::string getOutputPath(const std::string& imageFilePath) const override;

private:
    std::string m_executableLocation;

    std::string getArguments(
        const std::string& imageFilePath,
        const std::optional<Coordinates>& initialCoordinates, double fovW,
        double fovH, int regions) const;

    PlateSolveResult readResult(const std::string& outputFilePath,
                                int imageWidth, int imageHeight) const;
};
