#include "astrometry.hpp"

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/utils/cmdline.hpp"


AstrometrySolver::AstrometrySolver(const std::string &name) : Solver(name) {
    RegisterFunc("solveImage", &AstrometrySolver::_solveImage, this);
    RegisterFunc("getSolveResult", &AstrometrySolver::_getSolveResult, this);
    RegisterFunc("getSolveStatus", &AstrometrySolver::_getSolveStatus, this);
    RegisterFunc("setSolveParams", &AstrometrySolver::_setSolveParams, this);
    RegisterFunc("getSolveParams", &AstrometrySolver::_getSolveParams, this);

    RegisterFunc("connect", &AstrometrySolver::connect, this);
    RegisterFunc("disconnect", &AstrometrySolver::disconnect, this);
    RegisterFunc("reconnect", &AstrometrySolver::reconnect, this);
    RegisterFunc("isConnected", &AstrometrySolver::isConnected, this);

    DLOG_F(INFO, "Initializing Astrometry Solver...");
}

AstrometrySolver::~AstrometrySolver() {
    DLOG_F(INFO, "Destroying Astrometry Solver...");
}

bool AstrometrySolver::connect(const json &params) {
    DLOG_F(INFO, "Connecting to Astrometry Solver...");
    // Check the path
    if (!params.contains("path") || !params["path"].is_string()) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    std::string solverPath = params["path"].get<std::string>();
    if (!Atom::IO::isFileNameValid(solverPath) ||
        !Atom::IO::isFileExists(solverPath)) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    // Check whether the file is a executable file

    SetVariable("solverPath", params["path"].get<std::string>());
    DLOG_F(INFO, "Connected to Astrometry Solver");
    return true;
}

bool AstrometrySolver::disconnect(const json &params) {
    DLOG_F(INFO, "Disconnecting from Astrometry Solver...");
    SetVariable("solverPath", "");
    DLOG_F(INFO, "Disconnected from Astrometry Solver");
    return true;
}

bool AstrometrySolver::reconnect(const json &params) {
    DLOG_F(INFO, "Reconnecting to Astrometry Solver...");
    if (!disconnect(params)) {
        return false;
    }
    if (!connect(params)) {
        return false;
    }
    DLOG_F(INFO, "Reconnected to Astrometry Solver");
    return true;
}

bool AstrometrySolver::isConnected() {
    return GetVariable<std::string>("solverPath").has_value();
}

bool AstrometrySolver::solveImage(const std::string &image, const int &timeout,
                                  const bool &debug) {
    DLOG_F(INFO, "Solving Image {}...", image);
    if (!isConnected()) {
        LOG_F(ERROR, "Failed to execute {}: Not Connected", __func__);
        return false;
    }
    if (!Atom::IO::isFileNameValid(image) || !Atom::IO::isFileExists(image)) {
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__);
        return false;
    }
    SolveResult result;
    try {
        std::string command = makeCommand();

        std::string output = Atom::System::executeCommand(command, false);

        result = parseOutput(output);
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to execute {}: {}", __func__, e.what());
        return false;
    }
    // 将解析结果写入JSON对象
    if (!result.ra.empty()) {
        SetVariable("result.ra", result.ra);
    }
    if (!result.dec.empty()) {
        SetVariable("result.dec", result.dec);
    }
    if (result.fov_x > 0) {
        SetVariable("result.fov_x", result.fov_x);
    }
    if (result.fov_y > 0) {
        SetVariable("result.fov_y", result.fov_y);
    }
    if (result.rotation != 0) {
        SetVariable("result.rotation", result.rotation);
    }
    return true;
}

bool AstrometrySolver::getSolveResult(const int &timeout, const bool &debug) {
    DLOG_F(INFO, "Getting Solve Result...");
    return true;
}

bool AstrometrySolver::getSolveStatus(const int &timeout, const bool &debug) {
    DLOG_F(INFO, "Getting Solve Status...");
    return true;
}

bool AstrometrySolver::setSolveParams(const json &params) {
    DLOG_F(INFO, "Setting Solve Parameters...");
    bool status = true;
    if (params.contains("ra") && params["ra"].is_string()) {
        DLOG_F(INFO, "Setting Target RA {}", params["ra"].get<std::string>());
        status = SetVariable("target_ra", params["ra"].get<std::string>());
    }
    if (params.contains("dec") && params["dec"].is_string()) {
        DLOG_F(INFO, "Setting Target Dec {}", params["dec"].get<std::string>());
        status = SetVariable("target_dec", params["dec"].get<std::string>());
    }
    if (params.contains("radius") && params["radius"].is_number()) {
        DLOG_F(INFO, "Setting Search Radius {}",
               params["radius"].get<double>());
        status = SetVariable("radius", params["radius"].get<double>());
    }
    if (params.contains("downsample") && params["downsample"].is_number()) {
        DLOG_F(INFO, "Setting Downsample {}", params["downsample"].get<int>());
        status = SetVariable("downsample", params["downsample"].get<int>());
    }
    if (params.contains("depth") && params["depth"].is_array()) {
        for (auto &i : params["depth"].get<std::vector<int>>()) {
            DLOG_F(INFO, "Setting Depth {}", i);
            /// status = SetVariable("depth", i);
        }
        // status = SetVariable("depth_x",
        // params["depth"].get<std::vector<int>>());
    }
    if (params.contains("scale_low") && params["scale_low"].is_number()) {
        DLOG_F(INFO, "Setting Scale Low {}", params["scale_low"].get<double>());
        status = SetVariable("scale_low", params["scale_low"].get<double>());
    }
    if (params.contains("scale_high") && params["scale_high"].is_number()) {
        DLOG_F(INFO, "Setting Scale High {}",
               params["scale_high"].get<double>());
        status = SetVariable("scale_high", params["scale_high"].get<double>());
    }
    if (params.contains("width") && params["width"].is_number()) {
        DLOG_F(INFO, "Setting Width {}", params["width"].get<int>());
        status = SetVariable("width", params["width"].get<int>());
    }
    if (params.contains("height") && params["height"].is_number()) {
        DLOG_F(INFO, "Setting Height {}", params["height"].get<int>());
        status = SetVariable("height", params["height"].get<int>());
    }
    if (params.contains("scale_units") && params["scale_units"].is_string()) {
        DLOG_F(INFO, "Setting Scale Units {}",
               params["scale_units"].get<std::string>());
        status = SetVariable("scale_units",
                             params["scale_units"].get<std::string>());
    }
    if (params.contains("overwrite") && params["overwrite"].is_boolean()) {
        DLOG_F(INFO, "Setting Overwrite {}", params["overwrite"].get<bool>());
        status = SetVariable("overwrite", params["overwrite"].get<bool>());
    }
    if (params.contains("no_plot") && params["no_plot"].is_boolean()) {
        DLOG_F(INFO, "Setting No Plot {}", params["no_plot"].get<bool>());
        status = SetVariable("no_plot", params["no_plot"].get<bool>());
    }
    if (params.contains("verify") && params["verify"].is_boolean()) {
        DLOG_F(INFO, "Setting Verify {}", params["verify"].get<bool>());
        status = SetVariable("verify", params["verify"].get<bool>());
    }
    return status;
}

json AstrometrySolver::getSolveParams() { return json{}; }

std::string AstrometrySolver::makeCommand() {
    auto solverPath = GetVariable<std::string>("solverPath").value();
    auto image = GetVariable<std::string>("imagePath").value();
    auto ra = GetVariable<std::string>("target_ra").value();
    auto dec = GetVariable<std::string>("target_dec").value();
    auto radius = GetVariable<double>("radius").value();
    auto downsample = GetVariable<int>("downsample").value();
    auto depth = GetVariable<std::vector<int>>("depth").value();
    auto scale_low = GetVariable<double>("scale_low").value();
    auto scale_high = GetVariable<double>("scale_high").value();
    auto width = GetVariable<int>("width").value();
    auto height = GetVariable<int>("height").value();
    auto scale_units = GetVariable<std::string>("scale_units").value();
    auto overwrite = GetVariable<bool>("overwrite").value();
    auto no_plot = GetVariable<bool>("no_plot").value();
    auto verify = GetVariable<bool>("verify").value();

    // Max: Here we should use cmdline.hpp to make the command

    std::stringstream ss;
    ss << solverPath << " \"" << image << "\"";
    if (!ra.empty()) {
        ss << " --ra \"" << ra << "\"";
    }
    if (!dec.empty()) {
        ss << " --dec \"" << dec << "\"";
    }
    if (radius > 0) {
        ss << " --radius " << radius;
    }
    if (downsample != 1) {
        ss << " --downsample " << downsample;
    }
    if (!depth.empty()) {
        ss << " --depth " << depth[0] << "," << depth[1];
    }
    if (scale_low > 0) {
        ss << " --scale-low " << scale_low;
    }
    if (scale_high > 0) {
        ss << " --scale-high " << scale_high;
    }
    if (width > 0) {
        ss << " --width " << width;
    }
    if (height > 0) {
        ss << " --height " << height;
    }
    if (!scale_units.empty()) {
        ss << " --scale-units \"" << scale_units << "\"";
    }
    if (overwrite) {
        ss << " --overwrite";
    }
    if (no_plot) {
        ss << " --no-plot";
    }
    if (verify) {
        ss << " --verify";
    }

    std::string cmd = ss.str();
    DLOG_F(INFO, "Command: {}", cmd);
    return cmd;
}

SolveResult AstrometrySolver::parseOutput(const std::string &output) {
    SolveResult result;
    std::unordered_map<std::string, std::string> tokens;
    std::istringstream ss(output);
    std::string line;
    while (std::getline(ss, line)) {
        // 解析命令行输出结果中的每一行
        std::string key, value;
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
            result.fov_x = std::stod(iter->second.substr(0, pos));
            result.fov_y = std::stod(iter->second.substr(pos + 1));
        }
    }
    iter = tokens.find("Fieldrotationangleupisdegrees");
    if (iter != tokens.end()) {
        result.rotation = std::stod(iter->second);
    }

    return result;
}