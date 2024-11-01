#include "cmake.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

namespace lithium {

struct CMakeBuilder::Impl {
    CMakeBuilderConfig config;
};

namespace {
auto executeCommand(const std::string& command) -> BuildResult {
    int ret = atom::system::executeCommandWithStatus(command).second;
    if (ret != 0) {
        LOG_F(ERROR, "Command failed with exit code {}", ret);
        return BuildResult(false, "Command execution failed.", ret);
    }
    return BuildResult(true, "Command execution succeeded.", ret);
}

void logCommandExecution(const std::string& description,
                         const std::string& command) {
    LOG_F(INFO, "{}: {}", description, command);
}
}  // namespace

CMakeBuilder::CMakeBuilder() : pImpl_(std::make_unique<Impl>()) {}

CMakeBuilder::~CMakeBuilder() = default;

auto CMakeBuilder::configureProject(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options,
    const std::map<std::string, std::string>& envVars) -> BuildResult {
    LOG_F(INFO, "Configuring project: sourceDir={}, buildDir={}",
          sourceDir.string(), buildDir.string());

    std::ostringstream cmd;
    cmd << "cmake -S " << sourceDir.string() << " -B " << buildDir.string();

    // Append build type
    switch (buildType) {
        case BuildType::DEBUG:
            cmd << " -DCMAKE_BUILD_TYPE=Debug";
            break;
        case BuildType::RELEASE:
            cmd << " -DCMAKE_BUILD_TYPE=Release";
            break;
        case BuildType::REL_WITH_DEB_INFO:
            cmd << " -DCMAKE_BUILD_TYPE=RelWithDebInfo";
            break;
        case BuildType::MIN_SIZE_REL:
            cmd << " -DCMAKE_BUILD_TYPE=MinSizeRel";
            break;
    }

    // Append additional options
    for (const auto& opt : options) {
        cmd << " " << opt;
    }

    // Handle environment variables
    std::string envPrefix;
    for (const auto& [key, value] : envVars) {
        envPrefix.append(key).append("=").append(value).append(" ");
    }

    std::string fullCommand = envPrefix + cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::buildProject(const std::filesystem::path& buildDir,
                                std::optional<int> jobs) -> BuildResult {
    LOG_F(INFO, "Building project: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "cmake --build " << buildDir.string();
    if (jobs.has_value()) {
        cmd << " -- -j" << jobs.value();
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::cleanProject(const std::filesystem::path& buildDir)
    -> BuildResult {
    LOG_F(INFO, "Cleaning project: buildDir={}", buildDir.string());

    std::filesystem::remove_all(buildDir);

    LOG_F(INFO, "CMake clean succeeded.");
    return BuildResult(true, "CMake clean succeeded.", 0);
}

auto CMakeBuilder::installProject(const std::filesystem::path& buildDir,
                                  const std::filesystem::path& installDir)
    -> BuildResult {
    LOG_F(INFO, "Installing project: buildDir={}, installDir={}",
          buildDir.string(), installDir.string());

    std::ostringstream cmd;
    cmd << "cmake --install " << buildDir.string() << " --prefix "
        << installDir.string();

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::runTests(const std::filesystem::path& buildDir,
                            const std::vector<std::string>& testNames)
    -> BuildResult {
    LOG_F(INFO, "Running tests: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "ctest --test-dir " << buildDir.string();

    for (const auto& test : testNames) {
        cmd << " -R " << test;
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::generateDocs(const std::filesystem::path& buildDir,
                                const std::filesystem::path& outputDir)
    -> BuildResult {
    LOG_F(INFO, "Generating documentation: buildDir={}, outputDir={}",
          buildDir.string(), outputDir.string());

    std::ostringstream cmd;
    cmd << "doxygen " << (buildDir / "Doxyfile").string();

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::loadConfig(const std::filesystem::path& configPath) -> bool {
    LOG_F(INFO, "Loading configuration from {}", configPath.string());

    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            LOG_F(ERROR, "Failed to open configuration file: {}",
                  configPath.string());
            return false;
        }

        nlohmann::json configJson;
        configFile >> configJson;

        // Example: Assuming the JSON contains a key "buildType"
        if (configJson.contains("buildType")) {
            std::string buildTypeStr = configJson["buildType"];
            if (buildTypeStr == "Debug") {
                pImpl_->config.buildType = BuildType::DEBUG;
            } else if (buildTypeStr == "Release") {
                pImpl_->config.buildType = BuildType::RELEASE;
            } else if (buildTypeStr == "RelWithDebInfo") {
                pImpl_->config.buildType = BuildType::REL_WITH_DEB_INFO;
            } else if (buildTypeStr == "MinSizeRel") {
                pImpl_->config.buildType = BuildType::MIN_SIZE_REL;
            } else {
                LOG_F(ERROR, "Unknown build type: {}", buildTypeStr);
                return false;
            }
        } else {
            LOG_F(ERROR, "Configuration file missing 'buildType' key");
            return false;
        }

        // Example: Assuming the JSON contains a key "options"
        if (configJson.contains("options")) {
            pImpl_->config.options =
                configJson["options"].get<std::vector<std::string>>();
        } else {
            LOG_F(ERROR, "Configuration file missing 'options' key");
            return false;
        }

        // Example: Assuming the JSON contains a key "envVars"
        if (configJson.contains("envVars")) {
            pImpl_->config.envVars =
                configJson["envVars"].get<std::map<std::string, std::string>>();
        } else {
            LOG_F(ERROR, "Configuration file missing 'envVars' key");
            return false;
        }

        LOG_F(INFO, "Configuration loaded successfully.");
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception occurred while loading configuration: {}",
              e.what());
        return false;
    }
}

auto CMakeBuilder::getAvailableTargets(const std::filesystem::path& buildDir)
    -> std::vector<std::string> {
    LOG_F(INFO, "Retrieving available targets: buildDir={}", buildDir.string());

    std::string command =
        "cmake --build " + buildDir.string() + " --target help";
    auto output =
        atom::utils::splitString(atom::system::executeCommand(command), '\n');

    std::vector<std::string> targets;
    bool startParsing = false;
    for (const auto& line : output) {
        if (line.find("The following are some of the valid targets") !=
            std::string::npos) {
            startParsing = true;
            continue;
        }
        if (startParsing) {
            if (line.find("...") != std::string::npos) {
                break;
            }
            targets.push_back(line.substr(0, line.find_first_of(' ')));
        }
    }

    LOG_F(INFO, "Available targets retrieved: {}", targets.size());
    return targets;
}

auto CMakeBuilder::buildTarget(const std::filesystem::path& buildDir,
                               const std::string& target,
                               std::optional<int> jobs) -> BuildResult {
    LOG_F(INFO, "Building target: buildDir={}, target={}", buildDir.string(),
          target);

    std::ostringstream cmd;
    cmd << "cmake --build " << buildDir.string() << " --target " << target;
    if (jobs.has_value()) {
        cmd << " -- -j" << jobs.value();
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto CMakeBuilder::getCacheVariables(const std::filesystem::path& buildDir)
    -> std::vector<std::pair<std::string, std::string>> {
    LOG_F(INFO, "Retrieving cache variables: buildDir={}", buildDir.string());

    // Example: Parse CMake cache variables
    // Implementation omitted; returning empty list
    std::vector<std::pair<std::string, std::string>> cacheVars;

    LOG_F(INFO, "Cache variables retrieved.");
    return cacheVars;
}

auto CMakeBuilder::setCacheVariable(const std::filesystem::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) -> bool {
    LOG_F(INFO, "Setting cache variable: buildDir={}, name={}, value={}",
          buildDir.string(), name, value);

    std::ostringstream cmd;
    cmd << "cmake -S " << buildDir.string() << " -B " << buildDir.string()
        << " -D" << name << "=" << value;

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand).isSuccess();
}

}  // namespace lithium
