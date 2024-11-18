// astrometry.cpp

#include "astrometry.hpp"
#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

#include "tools/croods.hpp"
#include "tools/solverutils.hpp"

static auto astrometrySolver =
    std::make_shared<AstrometrySolver>("solver.astrometry");

AstrometrySolver::AstrometrySolver(std::string name) : name_(std::move(name)) {
    DLOG_F(INFO, "Initializing Astrometry Solver...");
}

AstrometrySolver::~AstrometrySolver() {
    DLOG_F(INFO, "Destroying Astrometry Solver...");
}

auto AstrometrySolver::connect(std::string_view solverPath) -> bool {
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

auto AstrometrySolver::disconnect() -> bool {
    DLOG_F(INFO, "Disconnecting from Astap Solver...");
    solverPath_.clear();
    DLOG_F(INFO, "Disconnected from Astap Solver");
    return true;
}

auto AstrometrySolver::reconnect() -> bool {
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

auto AstrometrySolver::isConnected() -> bool { return !solverPath_.empty(); }

auto AstrometrySolver::scanSolver() -> std::vector<std::string> {
    return atom::io::checkFileTypeInFolder("/usr/bin", {"solve-field"},
                                           atom::io::FileOption::NAME);
}

auto AstrometrySolver::solveImage(
    std::string_view image, std::optional<std::string_view> target_ra,
    std::optional<std::string_view> target_dec, std::optional<double> radius,
    std::optional<int> downsample, std::optional<int> depth, bool overWrite,
    bool noPlot, int timeout, int debug, const SolveOptions &options) -> bool {
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
        std::string command =
            buildCommand(image, target_ra, target_dec, radius, downsample,
                         depth, overWrite, noPlot, timeout, debug, options);
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
    std::istringstream outputStream(output);
    std::string line;
    while (std::getline(outputStream, line)) {
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
                           [](unsigned char ch) { return !std::isalnum(ch); }),
            key.end());
        tokens[key] = value;
    }

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
        result.rotation = iter->second;
    }

    return result;
}

std::string AstrometrySolver::buildCommand(
    std::string_view image, std::optional<std::string_view> target_ra,
    std::optional<std::string_view> target_dec, std::optional<double> radius,
    std::optional<int> downsample, std::optional<int> depth, bool overWrite,
    bool noPlot, int timeout, int debug, const SolveOptions &options) {
    std::vector<std::pair<std::string, std::optional<std::string>>> optionMap =
        {{"--backend-config", options.backend_config},
         {"--config", options.config},
         {"--batch",
          options.batch ? std::optional<std::string>("") : std::nullopt},
         {"--files-on-stdin", options.files_on_stdin
                                  ? std::optional<std::string>("")
                                  : std::nullopt},
         {"--no-plots",
          options.no_plots ? std::optional<std::string>("") : std::nullopt},
         {"--plot-scale",
          options.plot_scale
              ? std::make_optional(std::to_string(*options.plot_scale))
              : std::nullopt},
         {"--plot-bg", options.plot_bg},
         {"--use-wget",
          options.use_wget ? std::optional<std::string>("") : std::nullopt},
         {"--overwrite",
          options.overwrite ? std::optional<std::string>("") : std::nullopt},
         {"--continue",
          options.continue_run ? std::optional<std::string>("") : std::nullopt},
         {"--skip-solved",
          options.skip_solved ? std::optional<std::string>("") : std::nullopt},
         {"--fits-image",
          options.fits_image ? std::optional<std::string>("") : std::nullopt},
         {"--new-fits", options.new_fits},
         {"--kmz", options.kmz},
         {"--scamp", options.scamp},
         {"--scamp-config", options.scamp_config},
         {"--index-xyls", options.index_xyls},
         {"--just-augment",
          options.just_augment ? std::optional<std::string>("") : std::nullopt},
         {"--axy", options.axy},
         {"--temp-axy",
          options.temp_axy ? std::optional<std::string>("") : std::nullopt},
         {"--timestamp",
          options.timestamp ? std::optional<std::string>("") : std::nullopt},
         {"--no-delete-temp", options.no_delete_temp
                                  ? std::optional<std::string>("")
                                  : std::nullopt},
         {"--scale-low", options.scale_low ? std::make_optional(std::to_string(
                                                 *options.scale_low))
                                           : std::nullopt},
         {"--scale-high",
          options.scale_high
              ? std::make_optional(std::to_string(*options.scale_high))
              : std::nullopt},
         {"--scale-units", options.scale_units},
         {"--parity", options.parity},
         {"--code-tolerance",
          options.code_tolerance
              ? std::make_optional(std::to_string(*options.code_tolerance))
              : std::nullopt},
         {"--pixel-error",
          options.pixel_error
              ? std::make_optional(std::to_string(*options.pixel_error))
              : std::nullopt},
         {"--quad-size-min",
          options.quad_size_min
              ? std::make_optional(std::to_string(*options.quad_size_min))
              : std::nullopt},
         {"--quad-size-max",
          options.quad_size_max
              ? std::make_optional(std::to_string(*options.quad_size_max))
              : std::nullopt},
         {"--odds-to-tune-up",
          options.odds_to_tune_up
              ? std::make_optional(std::to_string(*options.odds_to_tune_up))
              : std::nullopt},
         {"--odds-to-solve",
          options.odds_to_solve
              ? std::make_optional(std::to_string(*options.odds_to_solve))
              : std::nullopt},
         {"--odds-to-reject",
          options.odds_to_reject
              ? std::make_optional(std::to_string(*options.odds_to_reject))
              : std::nullopt},
         {"--odds-to-stop-looking", options.odds_to_stop_looking
                                        ? std::make_optional(std::to_string(
                                              *options.odds_to_stop_looking))
                                        : std::nullopt},
         {"--use-source-extractor", options.use_source_extractor
                                        ? std::optional<std::string>("")
                                        : std::nullopt},
         {"--source-extractor-config", options.source_extractor_config},
         {"--source-extractor-path", options.source_extractor_path},
         {"--ra", options.ra},
         {"--dec", options.dec},
         {"--radius", options.radius
                          ? std::make_optional(std::to_string(*options.radius))
                          : std::nullopt},
         {"--depth", options.depth
                         ? std::make_optional(std::to_string(*options.depth))
                         : std::nullopt},
         {"--objs", options.objs
                        ? std::make_optional(std::to_string(*options.objs))
                        : std::nullopt},
         {"--cpulimit", options.cpulimit ? std::make_optional(std::to_string(
                                               *options.cpulimit))
                                         : std::nullopt},
         {"--resort",
          options.resort ? std::optional<std::string>("") : std::nullopt},
         {"--extension", options.extension ? std::make_optional(std::to_string(
                                                 *options.extension))
                                           : std::nullopt},
         {"--invert",
          options.invert ? std::optional<std::string>("") : std::nullopt},
         {"--downsample",
          options.downsample
              ? std::make_optional(std::to_string(*options.downsample))
              : std::nullopt},
         {"--no-background-subtraction", options.no_background_subtraction
                                             ? std::optional<std::string>("")
                                             : std::nullopt},
         {"--sigma", options.sigma
                         ? std::make_optional(std::to_string(*options.sigma))
                         : std::nullopt},
         {"--nsigma", options.nsigma
                          ? std::make_optional(std::to_string(*options.nsigma))
                          : std::nullopt},
         {"--no-remove-lines", options.no_remove_lines
                                   ? std::optional<std::string>("")
                                   : std::nullopt},
         {"--uniformize",
          options.uniformize
              ? std::make_optional(std::to_string(*options.uniformize))
              : std::nullopt},
         {"--no-verify-uniformize", options.no_verify_uniformize
                                        ? std::optional<std::string>("")
                                        : std::nullopt},
         {"--no-verify-dedup", options.no_verify_dedup
                                   ? std::optional<std::string>("")
                                   : std::nullopt},
         {"--cancel", options.cancel},
         {"--solved", options.solved},
         {"--solved-in", options.solved_in},
         {"--match", options.match},
         {"--rdls", options.rdls},
         {"--sort-rdls", options.sort_rdls},
         {"--tag", options.tag},
         {"--tag-all",
          options.tag_all ? std::optional<std::string>("") : std::nullopt},
         {"--scamp-ref", options.scamp_ref},
         {"--corr", options.corr},
         {"--wcs", options.wcs},
         {"--pnm", options.pnm},
         {"--keep-xylist", options.keep_xylist},
         {"--dont-augment",
          options.dont_augment ? std::optional<std::string>("") : std::nullopt},
         {"--verify", options.verify},
         {"--verify-ext", options.verify_ext},
         {"--no-verify",
          options.no_verify ? std::optional<std::string>("") : std::nullopt},
         {"--guess-scale",
          options.guess_scale ? std::optional<std::string>("") : std::nullopt},
         {"--crpix-center",
          options.crpix_center ? std::optional<std::string>("") : std::nullopt},
         {"--crpix-x",
          options.crpix_x ? std::make_optional(std::to_string(*options.crpix_x))
                          : std::nullopt},
         {"--crpix-y",
          options.crpix_y ? std::make_optional(std::to_string(*options.crpix_y))
                          : std::nullopt},
         {"--no-tweak",
          options.no_tweak ? std::optional<std::string>("") : std::nullopt},
         {"--tweak-order",
          options.tweak_order
              ? std::make_optional(std::to_string(*options.tweak_order))
              : std::nullopt},
         {"--predistort", options.predistort},
         {"--xscale", options.xscale
                          ? std::make_optional(std::to_string(*options.xscale))
                          : std::nullopt},
         {"--temp-dir", options.temp_dir}};

    std::vector<std::string> commandParts;
    commandParts.emplace_back(std::string(solverPath_) + " solve-field");

    for (const auto &[flag, value] : optionMap) {
        if (value.has_value()) {
            if (value.value().empty()) {
                commandParts.emplace_back(flag);
            } else {
                commandParts.emplace_back(flag + " " + value.value());
            }
        }
    }

    // 处理基本参数
    commandParts.emplace_back(std::string(image));

    // 处理 xyls 文件（如果有）
    // 根据需要添加处理逻辑

    // 拼接命令
    std::ostringstream commandStream;
    for (const auto &part : commandParts) {
        commandStream << part << " ";
    }

    return commandStream.str();
}

bool AstrometrySolver::plateSolveInProgress = false;
bool AstrometrySolver::isSolveImageFinished = false;

SolveResults AstrometrySolver::plateSolve(const std::string &filename,
                                          int focalLength,
                                          double cameraSizeWidth,
                                          double cameraSizeHeight) {
    plateSolveInProgress = true;
    isSolveImageFinished = false;
    SolveResults result;

    lithium::tools::MinMaxFOV fov = lithium::tools::calculateFOV(
        focalLength, cameraSizeWidth, cameraSizeHeight);

    std::stringstream cmd;
    cmd << "solve-field " << filename
        << " --overwrite --cpulimit 5 --scale-units degwidth"
        << " --scale-low " << fov.minFOV << " --scale-high " << fov.maxFOV
        << " --nsigma 8 --no-plots --no-remove-lines --uniformize 0 "
           "--timestamp";

    LOG_F(INFO, "Executing command: {}", cmd.str());
    std::string output = atom::system::executeCommand(cmd.str());

    return result;
}

auto AstrometrySolver::readSolveResult(const std::string &filename,
                                       int imageWidth,
                                       int imageHeight) -> SolveResults {
    isSolveImageFinished = false;
    SolveResults result;

    // Remove .fits extension
    std::string baseFilename = filename.substr(0, filename.length() - 5);

    std::string cmd = "wcsinfo " + baseFilename + ".wcs";
    LOG_F(INFO, "Executing command: {}", cmd);

    std::string output = atom::system::executeCommand(cmd);
    if (output.empty()) {
        LOG_F(ERROR, "Tools:Plate Solve Failure");
        result.RA_Degree = -1;
        result.DEC_Degree = -1;
        plateSolveInProgress = false;
        return result;
    }

    // Parse wcsinfo output
    size_t raPos = output.find("ra_center");
    size_t decPos = output.find("dec_center");
    size_t orientPos = output.find("orientation_center");
    size_t raHPos = output.find("ra_center_h");

    std::string raStr = output.substr(raPos + 10, decPos - raPos - 11);
    std::string decStr =
        output.substr(decPos + 11, orientPos - decPos - 12);
    std::string rotStr =
        output.substr(orientPos + 19, raHPos - orientPos - 20);

    double raDegree = std::stod(raStr);
    double decDegree = std::stod(decStr);
    double rotationDegree = std::stod(rotStr);

    lithium::tools::WCSParams wcs = lithium::tools::extractWCSParams(output);
    std::vector<lithium::tools::SphericalCoordinates> corners =
        getFOVCorners(wcs, imageWidth, imageHeight);

    result.RA_0 = corners[0].rightAscension;
    result.DEC_0 = corners[0].declination;
    result.RA_1 = corners[1].rightAscension;
    result.DEC_1 = corners[1].declination;
    result.RA_2 = corners[2].rightAscension;
    result.DEC_2 = corners[2].declination;
    result.RA_3 = corners[3].rightAscension;
    result.DEC_3 = corners[3].declination;

    result.RA_Degree = raDegree;
    result.DEC_Degree = decDegree;

    LOG_F(INFO, "Plate Solve Success");
    LOG_F(INFO, "RA: {} DEC: {}", raDegree, decDegree);

    plateSolveInProgress = false;
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
                  "Solve image with options");
    component.def("getSolveResult", &AstrometrySolver::getSolveResult, "main",
                  "Get solve result");

    component.addVariable("astrometry.instance", "Astap solver instance");
    component.defType<AstrometrySolver>("astrometry", "device.solver",
                                        "Astap solver");

    LOG_F(INFO, "Registered astrometry module.");
});