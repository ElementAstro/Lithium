#ifndef LITHIUM_CLIENT_ASTROMETRY_HPP
#define LITHIUM_CLIENT_ASTROMETRY_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

class SolveResult {
public:
    std::string ra;
    std::string dec;
    std::string rotation;
    double fovX;
    double fovY;
    double fovAvg;
    std::string error;
};

class AstrometrySolver {
public:
    explicit AstrometrySolver(std::string name);
    ~AstrometrySolver();

    auto connect(std::string_view solverPath) -> bool;

    auto disconnect() -> bool;

    auto reconnect() -> bool;

    auto isConnected() -> bool;

    auto scanSolver() -> std::vector<std::string>;

    auto solveImage(std::string_view image,
                    std::optional<std::string_view> target_ra,
                    std::optional<std::string_view> target_dec,
                    std::optional<double> radius, std::optional<int> downsample,
                    std::optional<int> depth, bool overWrite, bool noPlot,
                    int timeout, int debug) -> bool;

    auto getSolveResult(std::string_view image) -> SolveResult;

    auto readSolveResult(const std::string& ouput) -> SolveResult;

private:

    std::string name_;
    std::string solverPath_;
    std::string solverVersion_;

    SolveResult solveResult_;
};

#endif
