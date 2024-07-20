#include "astap.hpp"

#include "atom/async/async.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/system/software.hpp"
#include "macro.hpp"

#include <fitsio.h>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

AstapSolver::AstapSolver(std::string name) : name_(std::move(name)) {
    DLOG_F(INFO, "Initializing Astap Solver...");
    // Max: 当我们创建对象时，会自动扫描一次解析器的位置
    if (!scanSolver()) {
        LOG_F(ERROR, "Failed to execute {}: Astap not installed",
              ATOM_FUNC_NAME);
        return;
    }
}

AstapSolver::~AstapSolver() { DLOG_F(INFO, "Destroying Astap Solver..."); }

auto AstapSolver::connect(std::string_view solverPath) -> bool {
    if (solverPath.empty()) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }
    DLOG_F(INFO, "Connecting to Astap Solver...");
    if (!atom::io::isFileNameValid(solverPath.data()) ||
        !atom::io::isFileExists(solverPath)) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }
    solverPath_ = solverPath;
    DLOG_F(INFO, "Connected to Astap Solver");
    return true;
}

auto AstapSolver::disconnect() -> bool {
    DLOG_F(INFO, "Disconnecting from Astap Solver...");
    solverPath_.clear();
    DLOG_F(INFO, "Disconnected from Astap Solver");
    return true;
}

auto AstapSolver::reconnect() -> bool {
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

auto AstapSolver::isConnected() -> bool { return !solverPath_.empty(); }

auto AstapSolver::scanSolver() -> bool {
    DLOG_F(INFO, "Scanning Astap Solver...");
    if (isConnected()) {
        LOG_F(WARNING, "Solver is already connected");
    }
    auto isAstapCli = atom::system::checkSoftwareInstalled("astap-cli");
    if (!isAstapCli) {
        LOG_F(ERROR, "Failed to execute {}: Astap CLI not installed",
              ATOM_FUNC_NAME);
        return false;
    }
    auto astapCliPath = atom::system::getAppPath("astap-cli");
    if (!atom::io::isExecutableFile(astapCliPath, "astap-cli")) {
        LOG_F(ERROR, "Failed to execute {}: Astap not installed",
              ATOM_FUNC_NAME);
        return false;
    } else {
        solverPath_ = astapCliPath;
    }
    solverVersion_ = atom::system::getAppVersion(astapCliPath);
    if (solverVersion_.empty()) {
        LOG_F(ERROR, "Failed to execute {}: Astap version not got",
              ATOM_FUNC_NAME);
        return false;
    }
    LOG_F(INFO, "Current Astap version: {}", solverVersion_);
    return true;
}

auto AstapSolver::solveImage(std::string_view image,
                             std::optional<std::string_view> target_ra,
                             std::optional<std::string_view> target_dec,
                             std::optional<double> fov, bool update,
                             int timeout, int debug) -> bool {
    DLOG_F(INFO, "Solving Image {}...", image);
    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }
    if (!atom::io::isFileNameValid(image.data()) ||
        !atom::io::isFileExists(image)) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }
    SolveResult result;
    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image.data();

        if (target_ra.has_value()) {
            command << " -ra " << target_ra.value();
        }
        if (target_dec.has_value()) {
            command << " -dec " << target_dec.value();
        }
        if (fov.has_value()) {
            command << " -fov " << fov.value();
        }

        if (update) {
            command << " -update";
        }
        if (timeout != 0) {
            command << " -timeout " << timeout;
        }

        if (debug != 0) {
            command << " -debug";
        }

        LOG_F(INFO, "Executing command: {}", command.str());

        auto ret = atom::async::asyncRetry(
            [](const std::string &cmd) -> std::string {
                return atom::system::executeCommand(cmd, false);
            },
            3, std::chrono::seconds(5), command.str());

        auto startTime = std::chrono::system_clock::now();
        while (ret.wait_for(std::chrono::seconds(1)) !=
               std::future_status::ready) {
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now() - startTime)
                                   .count();
            if (elapsedTime > timeout) {
                LOG_F(ERROR, "Error: command timed out after {} seconds.",
                      timeout);
                return false;
            }
        }

        auto output = ret.get();
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);

        if (output.find("Solution found:") != std::string::npos) {
            DLOG_F(INFO, "Solved successfully");
            result = readSolveResult(image);
        } else {
            LOG_F(ERROR, "Failed to solve the image");
            result.error = "Failed to solve the image";
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, e.what());
        return false;
    }

    return true;
}

auto AstapSolver::getSolveResult(std::string_view image) -> SolveResult {
    DLOG_F(INFO, "Getting Solve Result...");
    return readSolveResult(image.data());
}

auto AstapSolver::readSolveResult(std::string_view image) -> SolveResult {
    SolveResult retStruct;
    fitsfile *fptr;
    int status = 0;
    fits_open_file(&fptr, image.data(), READONLY, &status);
    if (status != 0) {
        LOG_F(ERROR, "Failed to read FITS header: {}", image);
        return retStruct;
    }

    double solvedRa;
    double solvedDec;
    double xPixelArcsec;
    double yPixelArcsec;
    double rotation;
    double xPixelSize;
    double yPixelSize;
    char comment[FLEN_COMMENT];

    fits_read_key(fptr, TDOUBLE, "CRVAL1", &solvedRa, comment, &status);
    fits_read_key(fptr, TDOUBLE, "CRVAL2", &solvedDec, comment, &status);
    fits_read_key(fptr, TDOUBLE, "CDELT1", &xPixelArcsec, comment, &status);
    fits_read_key(fptr, TDOUBLE, "CDELT2", &yPixelArcsec, comment, &status);
    fits_read_key(fptr, TDOUBLE, "CROTA1", &rotation, comment, &status);
    fits_read_key(fptr, TDOUBLE, "XPIXSZ", &xPixelSize, comment, &status);
    fits_read_key(fptr, TDOUBLE, "YPIXSZ", &yPixelSize, comment, &status);

    fits_close_file(fptr, &status);
    if (status != 0) {
        LOG_F(ERROR, "Failed to close FITS file: {}", image);
        return retStruct;
    }

    retStruct.ra = std::to_string(solvedRa);
    retStruct.dec = std::to_string(solvedDec);
    retStruct.rotation = std::to_string(rotation);
    double xFocalLength = xPixelSize / xPixelArcsec * 206.625;
    double yFocalLength = yPixelSize / yPixelArcsec * 206.625;
    retStruct.fovX = xFocalLength;
    retStruct.fovY = yFocalLength;
    retStruct.fovAvg = (xFocalLength + yFocalLength) / 2.0;

    return retStruct;
}