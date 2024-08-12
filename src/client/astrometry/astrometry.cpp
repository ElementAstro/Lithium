#include "astrometry.hpp"
#include <memory>
#include <sstream>

#include "atom/components/registry.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

static auto astrometrySolver = std::make_shared<AstrometrySolver>("solver.astrometry");

AstrometrySolver::AstrometrySolver(std::string name) : name_(name) {
    DLOG_F(INFO, "Initializing Astrometry Solver...");
}

AstrometrySolver::~AstrometrySolver() {
    DLOG_F(INFO, "Destroying Astrometry Solver...");
}

bool AstrometrySolver::connect(std::string_view solverPath) {
    if (solverPath.empty()) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }
    DLOG_F(INFO, "Connecting to Astap Solver...");
    if (!atom::io::isFileNameValid(solverPath.data()) ||
        !atom::io::isFileExists(solverPath.data())) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }
    solverPath_ = solverPath;
    DLOG_F(INFO, "Connected to Astap Solver");
    return true;
}

bool AstrometrySolver::disconnect() {
    DLOG_F(INFO, "Disconnecting from Astap Solver...");
    solverPath_.clear();
    DLOG_F(INFO, "Disconnected from Astap Solver");
    return true;
}

bool AstrometrySolver::reconnect() {
    DLOG_F(INFO, "Reconnecting to Astap Solver...");
    std::string currentPath = solverPath_;
    if (!disconnect()) {
        return false;
    }
    if (!connect(currentPath)) {
        return false;
    }
    DLOG_F(INFO, "Reconnected to Astap Solver");
    return true;
}

bool AstrometrySolver::isConnected() { return !solverPath_.empty(); }

bool AstrometrySolver::solveImage(
    std::string_view image, std::optional<std::string_view> target_ra,
    std::optional<std::string_view> target_dec, std::optional<double> radius,
    std::optional<int> downsample, std::optional<int> depth, bool overWrite,
    bool noPlot, [[maybe_unused]] int timeout, [[maybe_unused]] int debug) {
    DLOG_F(INFO, "Solving Image {}...", image);
    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", __func__);
        return false;
    }
    if (!atom::io::isFileNameValid(image.data()) ||
        !atom::io::isFileExists(image.data())) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    try {
        std::ostringstream sss;
        sss << solverPath_;
        if (target_ra.has_value()) {
            sss << " --ra " << target_ra.value();
        }
        if (target_dec.has_value()) {
            sss << " --dec " << target_dec.value();
        }
        if (radius.has_value()) {
            sss << " --radius " << radius.value();
        }
        if (downsample.has_value()) {
            sss << " --downsample " << downsample.value();
        }
        if (depth.has_value()) {
            sss << " --depth " << depth.value();
        }
        if (overWrite) {
            sss << " --overwrite";
        }
        if (noPlot) {
            sss << " --no-plot";
        }
        std::string command = sss.str();
        std::string output = atom::system::executeCommand(command, false);

        solveResult_ = readSolveResult(output);
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to execute {}: {}", __func__, e.what());
        return false;
    }
    return true;
}

auto AstrometrySolver::getSolveResult(std::string_view /*image*/)
    -> SolveResult {
    DLOG_F(INFO, "Getting Solve Result...");
    return solveResult_;
}

auto AstrometrySolver::readSolveResult(const std::string &output)
    -> SolveResult {
    SolveResult result;
    std::unordered_map<std::string, std::string> tokens;
    std::istringstream ss(output);
    std::string line;
    while (std::getline(ss, line)) {
        // 解析命令行输出结果中的每一行
        std::string key;
        std::string value;
        auto pos = line.find(": ");
        if (pos != std::string::npos) {
            key = line.substr(0, pos);
            value = line.substr(pos + 2);
        } else {
            key = line;
            value = "";
        }
        key.erase(
            std::remove_if(key.begin(), key.end(),
                           [](unsigned char c) { return !std::isalnum(c); }),
            key.end());
        tokens[key] = value;
    }

    // 提取解析出的参数
    auto iter = tokens.find("FieldcenterRAHMSDecDMS");
    if (iter != tokens.end()) {
        auto pos = iter->second.find(",");
        if (pos != std::string::npos) {
            result.ra = iter->second.substr(0, pos);
            result.dec = iter->second.substr(pos + 1);
        }
    }
    iter = tokens.find("Fieldsize");
    if (iter != tokens.end()) {
        auto pos = iter->second.find("x");
        if (pos != std::string::npos) {
            result.fovX = std::stod(iter->second.substr(0, pos));
            result.fovY = std::stod(iter->second.substr(pos + 1));
            result.fovAvg = (result.fovX + result.fovY) / 2.0;
        }
    }
    iter = tokens.find("Fieldrotationangleupisdegrees");
    if (iter != tokens.end()) {
        result.rotation = std::stod(iter->second);
    }

    return result;
}

ATOM_MODULE(astrometry, [](Component &component) {
    LOG_F(INFO, "Registering astrometry module...");
    LOG_F(INFO, "AstrometryComponent Constructed");

    component.def("connect", &AstrometrySolver::connect, "main",
                  "Connect to astrometry solver");
    component.def("disconnect", &AstrometrySolver::disconnect, "main",
                  "Disconnect from astrometry solver");
    component.def("reconnect", &AstrometrySolver::reconnect, "main",
                  "Reconnect to astrometry solver");
    component.def("isConnected", &AstrometrySolver::isConnected, "main",
                  "Check if astrometry solver is connected");
    component.def("scanSolver", &AstrometrySolver::scanSolver, "main",
                  "Scan for astrometry solver");
    component.def("solveImage", &AstrometrySolver::solveImage, "main",
                  "Solve image");
    component.def("getSolveResult", &AstrometrySolver::getSolveResult, "main",
                  "Get solve result");

    component.addVariable("astrometry.instance", "Astap solver instance");
    component.defType<AstrometrySolver>("astrometry", "device.solver",
                                        "Astap solver");

    LOG_F(INFO, "Registered astrometry module.");
})
