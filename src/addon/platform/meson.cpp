#include "meson.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

namespace lithium {

struct MesonBuilder::Impl {
    MesonBuilderConfig config;
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

MesonBuilder::MesonBuilder() : pImpl_(std::make_unique<Impl>()) {}

MesonBuilder::~MesonBuilder() = default;

auto MesonBuilder::configureProject(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options,
    const std::map<std::string, std::string>& envVars) -> BuildResult {
    LOG_F(INFO, "Configuring project: sourceDir={}, buildDir={}",
          sourceDir.string(), buildDir.string());

    std::ostringstream cmd;
    cmd << "meson setup " << buildDir.string() << " " << sourceDir.string();

    // Append build type
    switch (buildType) {
        case BuildType::DEBUG:
            cmd << " --buildtype=debug";
            break;
        case BuildType::RELEASE:
            cmd << " --buildtype=release";
            break;
        case BuildType::REL_WITH_DEB_INFO:
            cmd << " --buildtype=debugoptimized";
            break;
        case BuildType::MIN_SIZE_REL:
            cmd << " --buildtype=release --strip -O3";
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

auto MesonBuilder::buildProject(const std::filesystem::path& buildDir,
                                std::optional<int> jobs) -> BuildResult {
    LOG_F(INFO, "Building project: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "meson compile -C " << buildDir.string();
    if (jobs.has_value()) {
        cmd << " -j" << jobs.value();
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::cleanProject(const std::filesystem::path& buildDir)
    -> BuildResult {
    LOG_F(INFO, "Cleaning project: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "meson compile -C " << buildDir.string() << " --clean";
    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::installProject(const std::filesystem::path& buildDir,
                                  const std::filesystem::path& installDir)
    -> BuildResult {
    LOG_F(INFO, "Installing project: buildDir={}, installDir={}",
          buildDir.string(), installDir.string());

    std::ostringstream cmd;
    cmd << "meson install -C " << buildDir.string() << " --destdir "
        << installDir.string();

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::runTests(const std::filesystem::path& buildDir,
                            const std::vector<std::string>& testNames)
    -> BuildResult {
    LOG_F(INFO, "Running tests: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "meson test -C " << buildDir.string();

    for (const auto& test : testNames) {
        cmd << " -t " << test;
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::generateDocs(const std::filesystem::path& buildDir,
                                const std::filesystem::path& outputDir)
    -> BuildResult {
    LOG_F(INFO, "Generating documentation: buildDir={}, outputDir={}",
          buildDir.string(), outputDir.string());

    std::ostringstream cmd;
    cmd << "sphinx-build -b html " << (buildDir / "docs").string() << " "
        << outputDir.string();

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::loadConfig(const std::filesystem::path& configPath) -> bool {
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

auto MesonBuilder::getAvailableTargets(const std::filesystem::path& buildDir)
    -> std::vector<std::string> {
    LOG_F(INFO, "Retrieving available targets: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "meson introspect --targets -C " << buildDir.string();
    auto output = atom::system::executeCommand(cmd.str());

    std::vector<std::string> targets;
    try {
        auto jsonOutput = nlohmann::json::parse(output);
        for (const auto& target : jsonOutput) {
            targets.push_back(target["name"]);
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to parse targets: {}", e.what());
    }

    LOG_F(INFO, "Available targets retrieved: {}", targets.size());
    return targets;
}

auto MesonBuilder::buildTarget(const std::filesystem::path& buildDir,
                               const std::string& target,
                               std::optional<int> jobs) -> BuildResult {
    LOG_F(INFO, "Building target: buildDir={}, target={}", buildDir.string(),
          target);

    std::ostringstream cmd;
    cmd << "meson compile -C " << buildDir.string() << " " << target;
    if (jobs.has_value()) {
        cmd << " -j" << jobs.value();
    }

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand);
}

auto MesonBuilder::getCacheVariables(const std::filesystem::path& buildDir)
    -> std::vector<std::pair<std::string, std::string>> {
    LOG_F(INFO, "Retrieving cache variables: buildDir={}", buildDir.string());

    std::ostringstream cmd;
    cmd << "meson configure -C " << buildDir.string();
    auto output = atom::system::executeCommand(cmd.str());

    std::vector<std::pair<std::string, std::string>> cacheVars;
    try {
        auto jsonOutput = nlohmann::json::parse(output);
        for (const auto& var : jsonOutput) {
            cacheVars.emplace_back(var["name"], var["value"]);
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to parse cache variables: {}", e.what());
    }

    LOG_F(INFO, "Cache variables retrieved.");
    return cacheVars;
}

auto MesonBuilder::setCacheVariable(const std::filesystem::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) -> bool {
    LOG_F(INFO, "Setting cache variable: buildDir={}, name={}, value={}",
          buildDir.string(), name, value);

    std::ostringstream cmd;
    cmd << "meson configure -C " << buildDir.string() << " -D" << name << "="
        << value;

    std::string fullCommand = cmd.str();
    logCommandExecution("Running command", fullCommand);
    return executeCommand(fullCommand).isSuccess();
}

}  // namespace lithium