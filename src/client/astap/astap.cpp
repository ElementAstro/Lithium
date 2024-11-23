#include "astap.hpp"

#include "atom/async/async.hpp"
#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/macro.hpp"
#include "atom/system/command.hpp"
#include "atom/system/software.hpp"
#include "device/template/solver.hpp"

#include <fitsio.h>
#include <array>
#include <chrono>
#include <cmath>
#include <optional>
#include <sstream>
#include <string_view>
#include <utility>

AstapSolver::AstapSolver(std::string name) : AtomSolver(std::move(name)) {
    DLOG_F(INFO, "Initializing Astap Solver...");
    if (!scanSolver()) {
        LOG_F(ERROR, "Failed to execute {}: Astap not installed",
              ATOM_FUNC_NAME);
        return;
    }
}

AstapSolver::~AstapSolver() { DLOG_F(INFO, "Destroying Astap Solver..."); }

auto AstapSolver::initialize() -> bool {
    DLOG_F(INFO, "Initializing Astap Solver...");
    return scanSolver();
}

auto AstapSolver::destroy() -> bool {
    DLOG_F(INFO, "Destroying Astap Solver...");
    // 添加必要的资源清理操作
    return true;
}

auto AstapSolver::connect(const std::string &name, int /*timeout*/,
                          int /*maxRetry*/) -> bool {
    if (name.empty() || !atom::io::isFileNameValid(name) ||
        !atom::io::isFileExists(name)) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }

    DLOG_F(INFO, "Connecting to Astap Solver...");
    solverPath_ = name;
    DLOG_F(INFO, "Connected to Astap Solver");
    return true;
}

auto AstapSolver::disconnect(bool /*force*/, int /*timeout*/,
                             int /*maxRetry*/) -> bool {
    DLOG_F(INFO, "Disconnecting from Astap Solver...");
    solverPath_.clear();
    DLOG_F(INFO, "Disconnected from Astap Solver");
    return true;
}

auto AstapSolver::reconnect(int timeout, int maxRetry) -> bool {
    DLOG_F(INFO, "Reconnecting to Astap Solver...");
    return disconnect(true, timeout, maxRetry) &&
           connect(solverPath_, timeout, maxRetry);
}

auto AstapSolver::isConnected() -> bool { return !solverPath_.empty(); }

auto AstapSolver::scanSolver() -> bool {
    DLOG_F(INFO, "Scanning Astap Solver...");
    if (isConnected()) {
        LOG_F(WARNING, "Solver is already connected");
        return true;
    }

    auto isAstapCli = atom::system::checkSoftwareInstalled("astap");
    if (!isAstapCli) {
        LOG_F(ERROR, "Failed to execute {}: Astap not installed",
              ATOM_FUNC_NAME);
        return false;
    }

    auto astapCliPath = atom::system::getAppPath("astap");
    if (!atom::io::isExecutableFile(astapCliPath.string(), "astap")) {
        LOG_F(ERROR, "Failed to execute {}: Astap not installed",
              ATOM_FUNC_NAME);
        return false;
    }

    solverPath_ = astapCliPath.string();
    solverVersion_ = atom::system::getAppVersion(astapCliPath.string());
    if (solverVersion_.empty()) {
        LOG_F(ERROR, "Failed to execute {}: Astap version not retrieved",
              ATOM_FUNC_NAME);
        return false;
    }

    LOG_F(INFO, "Current Astap version: {}", solverVersion_);
    return true;
}

auto AstapSolver::solve(const std::string &imageFilePath,
                        const std::optional<Coordinates> &initialCoordinates,
                        double fovW, double fovH, int imageWidth,
                        int imageHeight) -> PlateSolveResult {
    DLOG_F(INFO, "Solving Image {}...", imageFilePath);
    ATOM_UNREF_PARAM(imageFilePath);
    ATOM_UNREF_PARAM(initialCoordinates);
    ATOM_UNREF_PARAM(fovW);
    ATOM_UNREF_PARAM(fovH);
    ATOM_UNREF_PARAM(imageWidth);
    ATOM_UNREF_PARAM(imageHeight);
    return {};
}

auto AstapSolver::solveImage(
    std::string_view image, std::optional<double> radius_search_field,
    std::optional<double> field_height, std::optional<double> ra,
    std::optional<double> spd, std::optional<int> downsample_factor,
    std::optional<int> max_stars, std::optional<double> tolerance,
    std::optional<double> min_star_size, std::optional<bool> apply_check,
    std::optional<std::string> database_path,
    std::optional<std::string> database_abbreviation,
    std::optional<std::string> output_file, std::optional<bool> add_sip,
    std::optional<std::string> speed_mode, bool write_wcs, bool update,
    bool log, int timeout, int debug) -> bool {
    DLOG_F(INFO, "Solving Image {}...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    if (!atom::io::isFileNameValid(image.data()) ||
        !atom::io::isFileExists(image.data())) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",
              ATOM_FUNC_NAME);
        return false;
    }

    SolveResult result;
    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image;

        if (radius_search_field) {
            command << " -r " << *radius_search_field;
        }
        if (field_height) {
            command << " -fov " << *field_height;
        }
        if (ra) {
            command << " -ra " << *ra;
        }
        if (spd) {
            command << " -spd " << *spd;
        }
        if (downsample_factor) {
            command << " -z " << *downsample_factor;
        }
        if (max_stars) {
            command << " -s " << *max_stars;
        }
        if (tolerance) {
            command << " -t " << *tolerance;
        }
        if (min_star_size) {
            command << " -m " << *min_star_size;
        }
        if (apply_check) {
            command << " -check " << (*apply_check ? "y" : "n");
        }
        if (database_path) {
            command << " -d " << *database_path;
        }
        if (database_abbreviation) {
            command << " -D " << *database_abbreviation;
        }
        if (output_file) {
            command << " -o " << *output_file;
        }
        if (add_sip) {
            command << " -sip " << (*add_sip ? "y" : "n");
        }
        if (speed_mode) {
            command << " -speed " << *speed_mode;
        }
        if (write_wcs) {
            command << " -wcs";
        }
        if (update) {
            command << " -update";
        }
        if (log) {
            command << " -log";
        }
        if (debug != 0) {
            command << " -debug";
        }

        LOG_F(INFO, "Executing command: {}", command.str());

        auto ret = atom::async::asyncRetry(
            [](const std::string &cmd) -> std::string {
                return atom::system::executeCommand(cmd, false);
            },
            3, std::chrono::milliseconds(5000),
            atom::async::BackoffStrategy::EXPONENTIAL,
            std::chrono::milliseconds(timeout * 1000),
            [] { LOG_F(INFO, "Retrying command..."); },
            [](const std::exception &ex) {
                LOG_F(ERROR, "Exception: {}", ex.what());
            },
            [] { LOG_F(INFO, "Retry complete."); }, command.str());

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
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::analyseImage(std::string_view image, double snr_minimum,
                               bool extract, bool extract2) -> bool {
    DLOG_F(INFO, "Analysing Image {}...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image;

        if (extract2) {
            command << " -extract2 " << snr_minimum;
        } else if (extract) {
            command << " -extract " << snr_minimum;
        } else {
            command << " -analyse " << snr_minimum;
        }

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::convertToFits(std::string_view image, int binning) -> bool {
    DLOG_F(INFO, "Converting Image {} to FITS...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image << " -tofits " << binning;

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::annotateImage(std::string_view image) -> bool {
    DLOG_F(INFO, "Annotating Image {}...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image << " -annotate";

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::debugSolve(std::string_view image) -> bool {
    DLOG_F(INFO, "Debug Solving Image {}...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image << " -debug";

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::measureSkyBackground(std::string_view image,
                                       double pedestal) -> bool {
    DLOG_F(INFO, "Measuring Sky Background for Image {}...", image);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -f " << image << " -sqm " << pedestal;

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::findBestFocus(const std::vector<std::string> &image_files)
    -> bool {
    DLOG_F(INFO, "Finding Best Focus...");

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    if (image_files.size() < 4) {
        LOG_F(ERROR, "At least four images are required for focus analysis");
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_;
        for (size_t i = 0; i < image_files.size(); ++i) {
            command << " -focus" << (i + 1) << " " << image_files[i];
        }

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), false);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::liveStack(std::string_view path) -> bool {
    DLOG_F(INFO, "Starting Live Stack at Path {}...", path);

    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", ATOM_FUNC_NAME);
        return false;
    }

    try {
        std::ostringstream command;
        command << solverPath_ << " -stack";
        if (!path.empty()) {
            command << " \"" << path << "\"";
        }

        LOG_F(INFO, "Executing command: {}", command.str());

        auto output = atom::system::executeCommand(command.str(), true);
        DLOG_F(INFO, "Command '{}' returned: {}", command.str(), output);
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "Failed to execute {}: {}", ATOM_FUNC_NAME, ex.what());
        return false;
    }

    return true;
}

auto AstapSolver::getSolveResult(std::string_view image) -> SolveResult {
    DLOG_F(INFO, "Getting Solve Result...");
    return readSolveResult(image);
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
    std::array<char, FLEN_COMMENT> comment;

    fits_read_key(fptr, TDOUBLE, "CRVAL1", &solvedRa, comment.data(), &status);
    fits_read_key(fptr, TDOUBLE, "CRVAL2", &solvedDec, comment.data(), &status);
    fits_read_key(fptr, TDOUBLE, "CDELT1", &xPixelArcsec, comment.data(),
                  &status);
    fits_read_key(fptr, TDOUBLE, "CDELT2", &yPixelArcsec, comment.data(),
                  &status);
    fits_read_key(fptr, TDOUBLE, "CROTA1", &rotation, comment.data(), &status);
    fits_read_key(fptr, TDOUBLE, "XPIXSZ", &xPixelSize, comment.data(),
                  &status);
    fits_read_key(fptr, TDOUBLE, "YPIXSZ", &yPixelSize, comment.data(),
                  &status);

    fits_close_file(fptr, &status);
    if (status != 0) {
        LOG_F(ERROR, "Failed to close FITS file: {}", image);
        return retStruct;
    }

    retStruct.ra = std::to_string(solvedRa);
    retStruct.dec = std::to_string(solvedDec);
    retStruct.rotation = std::to_string(rotation);

    constexpr double arcsecToRad = 206.625;
    double xFocalLength = xPixelSize / xPixelArcsec * arcsecToRad;
    double yFocalLength = yPixelSize / yPixelArcsec * arcsecToRad;

    retStruct.fovX = xFocalLength;
    retStruct.fovY = yFocalLength;
    retStruct.fovAvg = (xFocalLength + yFocalLength) / 2.0;

    return retStruct;
}

ATOM_MODULE(solver_astap, [](Component &component) {
    LOG_F(INFO, "Registering solver_astap module...");

    component.def("connect", &AstapSolver::connect, "main",
                  "Connect to astap solver");
    component.def("disconnect", &AstapSolver::disconnect, "main",
                  "Disconnect from astap solver");
    component.def("reconnect", &AstapSolver::reconnect, "main",
                  "Reconnect to astap solver");
    component.def("isConnected", &AstapSolver::isConnected, "main",
                  "Check if astap solver is connected");
    component.def("scanSolver", &AstapSolver::scanSolver, "main",
                  "Scan for astap solver");
    component.def("solveImage", &AstapSolver::solveImage, "main",
                  "Solve image with various options");
    component.def("analyseImage", &AstapSolver::analyseImage, "main",
                  "Analyse image and report HFD");
    component.def("convertToFits", &AstapSolver::convertToFits, "main",
                  "Convert image to FITS format");
    component.def("annotateImage", &AstapSolver::annotateImage, "main",
                  "Annotate image with deep sky objects");
    component.def("debugSolve", &AstapSolver::debugSolve, "main",
                  "Show GUI and stop prior to solving");
    component.def("measureSkyBackground", &AstapSolver::measureSkyBackground,
                  "main", "Measure sky background value");
    component.def("findBestFocus", &AstapSolver::findBestFocus, "main",
                  "Find best focus point using curve fitting");
    component.def("liveStack", &AstapSolver::liveStack, "main",
                  "Start ASTAP with live stack tab visible");
    component.def("getSolveResult", &AstapSolver::getSolveResult, "main",
                  "Get solve result");

    component.addVariable("astap.instance", "Astap solver instance");
    component.defType<AstapSolver>("astap");

    component.def(
        "create_instance",
        [](const std::string &name) {
            std::shared_ptr<AtomSolver> instance =
                std::make_shared<AstapSolver>(name);
            return instance;
        },
        "device", "Create a new solver instance.");
    component.defType<AstapSolver>("solver.astap", "device",
                                   "Define a new solver instance.");

    LOG_F(INFO, "Registered solver_astap module.");
});
