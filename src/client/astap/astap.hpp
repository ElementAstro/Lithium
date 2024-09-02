#ifndef LITHIUM_CLIENT_ASTAP_HPP
#define LITHIUM_CLIENT_ASTAP_HPP

#include "device/template/solver.hpp"

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

class AstapSolver : public AtomSolver {
public:
    explicit AstapSolver(std::string name);
    ~AstapSolver();

    auto initialize() -> bool override;

    auto destroy() -> bool override;

    auto connect(const std::string& name, int timeout,
                 int maxRetry) -> bool override;

    auto disconnect(bool force, int timeout, int maxRetry) -> bool override;

    auto reconnect(int timeout, int maxRetry) -> bool override;

    auto scan() -> std::vector<std::string> override;

    auto isConnected() -> bool override;

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
