#include "xmake.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace lithium {

class XMakeBuilderImpl {
public:
    std::unique_ptr<json> configOptions = std::make_unique<json>();
    std::vector<std::string> dependencies;
};

XMakeBuilder::XMakeBuilder() : pImpl_(std::make_unique<XMakeBuilderImpl>()) {}
XMakeBuilder::~XMakeBuilder() = default;

auto XMakeBuilder::checkAndInstallDependencies() -> bool {
    for (const auto& dep : pImpl_->dependencies) {
        std::string checkCommand = "pkg-config --exists " + dep;
        if (!atom::system::executeCommandSimple(checkCommand)) {
            LOG_F(INFO, "Dependency {} not found, attempting to install...",
                  dep);
            std::string installCommand = "sudo apt-get install -y " + dep;
            if (!atom::system::executeCommandSimple(installCommand)) {
                LOG_F(ERROR, "Failed to install dependency: {}", dep);
                return false;
            }
        }
    }
    return true;
}

auto XMakeBuilder::configureProject(
    const fs::path& sourceDir, const fs::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options) -> BuildResult {
    BuildResult result;
    if (!fs::exists(buildDir)) {
        fs::create_directories(buildDir);
    }

    if (!checkAndInstallDependencies()) {
        result.success = false;
        result.error = "Failed to install dependencies.";
        return result;
    }

    std::string buildTypeStr;
    switch (buildType) {
        case BuildType::DEBUG:
            buildTypeStr = "debug";
            break;
        case BuildType::RELEASE:
            buildTypeStr = "release";
            break;
        case BuildType::REL_WITH_DEB_INFO:
            buildTypeStr = "reldebug";
            break;
        case BuildType::MIN_SIZE_REL:
            buildTypeStr = "minsizerel";
            break;
    }

    std::string opts;
    for (const auto& opt : options) {
        opts += " " + opt;
    }

    std::string command =
        "xmake f -p " + buildTypeStr + " -o " + buildDir.string() + " " + opts;
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Configured successfully.";
    } else {
        result.success = false;
        result.error = "Configuration failed.";
    }
    return result;
}

auto XMakeBuilder::buildProject(const fs::path& buildDir,
                                std::optional<int> jobs) -> BuildResult {
    BuildResult result;
    std::string command = "xmake -C " + buildDir.string();
    if (jobs && *jobs > 0) {
        command += " -j" + std::to_string(*jobs);
    }
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Build succeeded.";
    } else {
        result.success = false;
        result.error = "Build failed.";
    }
    return result;
}

auto XMakeBuilder::cleanProject(const fs::path& buildDir) -> BuildResult {
    BuildResult result;
    if (!fs::exists(buildDir)) {
        result.success = false;
        result.error = "Build directory does not exist: " + buildDir.string();
        return result;
    }
    std::string command = "xmake clean -C " + buildDir.string();
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Clean succeeded.";
    } else {
        result.success = false;
        result.error = "Clean failed.";
    }
    return result;
}

auto XMakeBuilder::installProject(const fs::path& buildDir,
                                  const fs::path& installDir) -> BuildResult {
    BuildResult result;
    std::string command =
        "xmake install -o " + installDir.string() + " -C " + buildDir.string();
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Install succeeded.";
    } else {
        result.success = false;
        result.error = "Install failed.";
    }
    return result;
}

auto XMakeBuilder::runTests(const fs::path& buildDir,
                            const std::vector<std::string>& testNames)
    -> BuildResult {
    BuildResult result;
    std::string command = "xmake run test -C " + buildDir.string();
    for (const auto& testName : testNames) {
        command += " " + testName;
    }
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Tests ran successfully.";
    } else {
        result.success = false;
        result.error = "Tests failed.";
    }
    return result;
}

auto XMakeBuilder::generateDocs(const fs::path& buildDir,
                                const fs::path& outputDir) -> BuildResult {
    BuildResult result;
    if (std::string command =
            "xmake doc -C " + buildDir.string() + " -o " + outputDir.string();
        atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Documentation generated successfully.";
    } else {
        result.success = false;
        result.error = "Documentation generation failed.";
    }
    return result;
}

auto XMakeBuilder::loadConfig(const fs::path& configPath) -> bool {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        LOG_F(ERROR, "Failed to open config file: {}", configPath.string());
        return false;
    }

    try {
        configFile >> *(pImpl_->configOptions);
        pImpl_->dependencies = pImpl_->configOptions->at("dependencies")
                                   .get<std::vector<std::string>>();
    } catch (const json::parse_error& e) {
        LOG_F(ERROR, "Failed to parse config file: {} with {}",
              configPath.string(), e.what());
        return false;
    } catch (const json::type_error& e) {
        LOG_F(ERROR, "Failed to parse config file: {} with {}",
              configPath.string(), e.what());
        return false;
    }

    configFile.close();
    return true;
}

auto XMakeBuilder::setLogCallback(
    [[maybe_unused]] std::function<void(const std::string&)> callback) -> void {
    // TODO: Set the log callback function
}

auto XMakeBuilder::getAvailableTargets(const fs::path& buildDir)
    -> std::vector<std::string> {
    std::vector<std::string> targets;
    std::string command = "xmake show -C " + buildDir.string();
    std::string output;
    if (const auto [output, status] =
            atom::system::executeCommandWithStatus(command);
        status == 0) {
        // Assume that the output contains targets in a specific format, e.g.,
        // each target on a new line
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            targets.push_back(line);
        }
    } else {
        LOG_F(ERROR, "Failed to retrieve available targets.");
    }
    return targets;
}

auto XMakeBuilder::buildTarget(const fs::path& buildDir,
                               const std::string& target,
                               std::optional<int> jobs) -> BuildResult {
    BuildResult result;
    std::string command = "xmake build " + target + " -C " + buildDir.string();
    if (jobs && *jobs > 0) {
        command += " -j" + std::to_string(*jobs);
    }
    if (atom::system::executeCommandSimple(command)) {
        result.success = true;
        result.output = "Target " + target + " built successfully.";
    } else {
        result.success = false;
        result.error = "Failed to build target: " + target;
    }
    return result;
}

auto XMakeBuilder::getCacheVariables(const fs::path& buildDir)
    -> std::vector<std::pair<std::string, std::string>> {
    std::vector<std::pair<std::string, std::string>> cacheVariables;
    std::string command = "xmake config --show -C " + buildDir.string();
    std::string output;
    if (const auto& [output, status] =
            atom::system::executeCommandWithStatus(command);
        status == 0) {
        // Parse the output to extract cache variables, assuming a key=value
        // format
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                cacheVariables.emplace_back(key, value);
            }
        }
    } else {
        LOG_F(ERROR, "Failed to retrieve cache variables.");
    }
    return cacheVariables;
}

auto XMakeBuilder::setCacheVariable(const fs::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) -> bool {
    std::string command =
        "xmake config " + name + "=" + value + " -C " + buildDir.string();
    if (atom::system::executeCommandSimple(command)) {
        LOG_F(INFO, "Cache variable {} set to {}.", name, value);
        return true;
    } else {
        LOG_F(ERROR, "Failed to set cache variable: {}", name);
        return false;
    }
}

}  // namespace lithium
