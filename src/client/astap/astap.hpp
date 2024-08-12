#ifndef LITHIUM_CLIENT_ASTAP_HPP
#define LITHIUM_CLIENT_ASTAP_HPP

#include <optional>
#include <string>
#include <string_view>

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

class AstapSolver {
public:
    explicit AstapSolver(std::string name);
    ~AstapSolver();

    auto connect(std::string_view solverPath) -> bool;

    auto disconnect() -> bool;

    auto reconnect() -> bool;

    auto isConnected() -> bool;

    auto scanSolver() -> bool;

    auto solveImage(std::string_view image,
                    std::optional<std::string_view> target_ra,
                    std::optional<std::string_view> target_dec,
                    std::optional<double> fov, bool update, int timeout,
                    int debug) -> bool;

    auto getSolveResult(std::string_view image) -> SolveResult;

private:
    auto readSolveResult(std::string_view image) -> SolveResult;

    std::string name_;
    std::string solverPath_;
    std::string solverVersion_;
};

#endif
