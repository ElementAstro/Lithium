#include "meson.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"
namespace fs = std::filesystem;
using json = nlohmann::json;

namespace lithium {
class MesonBuilderImpl {
public:
    std::unique_ptr<json> configOptions = std::make_unique<json>();
    std::vector<std::string> preBuildScripts;
    std::vector<std::string> postBuildScripts;
    std::vector<std::string> environmentVariables;
    std::vector<std::string> dependencies;
};

MesonBuilder::MesonBuilder() : pImpl_(std::make_unique<MesonBuilderImpl>()) {}
MesonBuilder::~MesonBuilder() = default;

auto MesonBuilder::checkAndInstallDependencies() -> bool {
    for (const auto &dep : pImpl_->dependencies) {
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

auto MesonBuilder::configureProject(
    const std::string &sourceDir, const std::string &buildDir,
    const std::string &buildType,
    const std::vector<std::string> &options) -> bool {
    if (!fs::exists(buildDir)) {
        fs::create_directories(buildDir);
    }

    if (!checkAndInstallDependencies()) {
        return false;
    }

    std::string opts;
    for (const auto &opt : options) {
        opts += " " + opt;
    }

    std::string command = "meson setup " + buildDir + " " + sourceDir +
                          " --buildtype=" + buildType + opts;
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::buildProject(const std::string &buildDir, int jobs) -> bool {
    std::string command = "ninja -C " + buildDir;
    if (jobs > 0) {
        command += " -j" + std::to_string(jobs);
    }
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::cleanProject(const std::string &buildDir) -> bool {
    if (!fs::exists(buildDir)) {
        LOG_F(ERROR, "Build directory does not exist: {}", buildDir);
        return false;
    }
    std::string command = "ninja -C " + buildDir + " clean";
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::installProject(const std::string &buildDir,
                                  const std::string &installDir) -> bool {
    std::string command =
        "ninja -C " + buildDir + " install --prefix " + installDir;
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::runTests(const std::string &buildDir) -> bool {
    std::string command = "meson test -C " + buildDir;
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::generateDocs(const std::string &buildDir) -> bool {
    std::string command = "ninja -C " + buildDir + " docs";
    return atom::system::executeCommandSimple(command);
}

auto MesonBuilder::loadConfig(const std::string &configPath) -> bool {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        LOG_F(ERROR, "Failed to open config file: {}", configPath);
        return false;
    }

    try {
        configFile >> *(pImpl_->configOptions);
        pImpl_->preBuildScripts = pImpl_->configOptions->at("preBuildScripts")
                                      .get<std::vector<std::string>>();
        pImpl_->postBuildScripts = pImpl_->configOptions->at("postBuildScripts")
                                       .get<std::vector<std::string>>();
        pImpl_->environmentVariables =
            pImpl_->configOptions->at("environmentVariables")
                .get<std::vector<std::string>>();
        pImpl_->dependencies = pImpl_->configOptions->at("dependencies")
                                   .get<std::vector<std::string>>();
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "Failed to parse config file: {} with {}", configPath,
              e.what());
        return false;
    } catch (const json::type_error &e) {
        LOG_F(ERROR, "Failed to parse config file: {} with {}", configPath,
              e.what());
        return false;
    }

    configFile.close();
    return true;
}

}  // namespace lithium
