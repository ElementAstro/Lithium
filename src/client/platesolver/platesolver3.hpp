#pragma once

#include <memory>
#include "device/template/solver.hpp"

class Platesolve3Solver : public AtomSolver {
public:
    explicit Platesolve3Solver(std::string executableLocation);
    ~Platesolve3Solver();

    PlateSolveResult solve(const std::string& imageFilePath,
                           const std::optional<Coordinates>& initialCoordinates,
                           double fovW, double fovH, int imageWidth,
                           int imageHeight) override;

protected:
    std::string getOutputPath(const std::string& imageFilePath) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    Platesolve3Solver(const Platesolve3Solver&) = delete;
    Platesolve3Solver& operator=(const Platesolve3Solver&) = delete;
};