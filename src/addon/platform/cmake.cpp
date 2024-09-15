#include "cmake.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>

#include "addon/platform/base.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace lithium {

class CMakeBuilderImpl {
public:
    std::unique_ptr<json> configOptions = std::make_unique<json>();
    std::vector<std::string> preBuildScripts;
    std::vector<std::string> postBuildScripts;
    std::vector<std::string> environmentVariables;
    std::vector<std::string> dependencies;
    std::function<void(const std::string&)> logCallback;
};

CMakeBuilder::CMakeBuilder() : pImpl_(std::make_unique<CMakeBuilderImpl>()) {}
CMakeBuilder::~CMakeBuilder() = default;

auto CMakeBuilder::checkAndInstallDependencies() -> bool {
    for (const auto& dep : pImpl_->dependencies) {
        std::string checkCommand = "pkg-config --exists " + dep;
        if (!atom::system::executeCommandSimple(checkCommand)) {
            pImpl_->logCallback("Dependency " + dep +
                                " not found, attempting to install...");
            std::string installCommand = "sudo apt-get install -y " + dep;
            if (!atom::system::executeCommandSimple(installCommand)) {
                pImpl_->logCallback("Failed to install dependency: " + dep);
                return false;
            }
        }
    }
    return true;
}

auto CMakeBuilder::configureProject(
    const std::filesystem::path& sourceDir,
    const std::filesystem::path& buildDir, BuildType buildType,
    const std::vector<std::string>& options) -> BuildResult {
    if (!fs::exists(buildDir)) {
        fs::create_directories(buildDir);
    }

    if (!checkAndInstallDependencies()) {
        return {false, "", "Failed to install dependencies"};
    }

    std::string buildTypeStr;
    switch (buildType) {
        case BuildType::Debug:
            buildTypeStr = "Debug";
            break;
        case BuildType::Release:
            buildTypeStr = "Release";
            break;
        case BuildType::RelWithDebInfo:
            buildTypeStr = "RelWithDebInfo";
            break;
        case BuildType::MinSizeRel:
            buildTypeStr = "MinSizeRel";
            break;
    }

    std::string command = "cmake -S " + sourceDir.string() + " -B " +
                          buildDir.string() +
                          " -DCMAKE_BUILD_TYPE=" + buildTypeStr;

    for (const auto& opt : options) {
        command += " " + opt;
    }

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to configure project.";
    }
    return res;
}

auto CMakeBuilder::buildProject(const std::filesystem::path& buildDir,
                                std::optional<int> jobs) -> BuildResult {
    std::string command = "cmake --build " + buildDir.string();
    if (jobs.has_value()) {
        command += " -j" + std::to_string(*jobs);
    }

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to build project.";
    }
    return res;
}

auto CMakeBuilder::cleanProject(const std::filesystem::path& buildDir)
    -> BuildResult {
    if (!fs::exists(buildDir)) {
        return {false, "",
                "Build directory does not exist: " + buildDir.string()};
    }

    std::string command =
        "cmake --build " + buildDir.string() + " --target clean";
    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to clean project.";
    }
    return res;
}

auto CMakeBuilder::installProject(const std::filesystem::path& buildDir,
                                  const std::filesystem::path& installDir)
    -> BuildResult {
    std::string command = "cmake --install " + buildDir.string() +
                          " --prefix " + installDir.string();

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to install project.";
    }
    return res;
}

auto CMakeBuilder::generateDocs(const std::filesystem::path& buildDir,
                                const std::filesystem::path& outputDir)
    -> BuildResult {
    std::string command =
        "cmake --build " + buildDir.string() + " --target docs";
    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        fs::path docsDir = buildDir / "docs";
        if (fs::exists(docsDir)) {
            fs::create_directories(outputDir);
            fs::copy(docsDir, outputDir,
                     fs::copy_options::recursive |
                         fs::copy_options::update_existing);
            res.output =
                "Documentation generated and copied to " + outputDir.string();
        } else {
            res.error = "Documentation directory not found after generation.";
        }
    } else {
        res.error = "Failed to generate documentation.";
    }
    return res;
}

auto CMakeBuilder::buildTarget(const std::filesystem::path& buildDir,
                               const std::string& target,
                               std::optional<int> jobs) -> BuildResult {
    std::string command =
        "cmake --build " + buildDir.string() + " --target " + target;
    if (jobs.has_value()) {
        command += " -j" + std::to_string(*jobs);
    }

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to build target: " + target;
    }
    return res;
}

auto CMakeBuilder::setCacheVariable(const std::filesystem::path& buildDir,
                                    const std::string& name,
                                    const std::string& value) -> bool {
    std::string command =
        "cmake -D" + name + "=" + value + " " + buildDir.string();

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    return status == 0;
}

auto CMakeBuilder::runTests(const std::filesystem::path& buildDir,
                            const std::vector<std::string>& testNames)
    -> BuildResult {
    std::string command = "ctest --test-dir " + buildDir.string();
    if (!testNames.empty()) {
        command +=
            " -R \"" +
            std::accumulate(testNames.begin(), testNames.end(), std::string(),
                            [](const std::string& a, const std::string& b) {
                                return a + (a.empty() ? "" : "|") + b;
                            }) +
            "\"";
    }

    auto [output, status] = atom::system::executeCommandWithStatus(command);
    BuildResult res;
    res.success = status == 0;
    if (status == 0) {
        res.output = output;
    } else {
        res.error = "Failed to run tests.";
    }
    return res;
}

auto CMakeBuilder::loadConfig(const std::filesystem::path& configPath) -> bool {
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        pImpl_->logCallback("Failed to open config file: " +
                            configPath.string());
        return false;
    }

    try {
        configFile >> *(pImpl_->configOptions);
        pImpl_->preBuildScripts = pImpl_->configOptions->value(
            "preBuildScripts", std::vector<std::string>{});
        pImpl_->postBuildScripts = pImpl_->configOptions->value(
            "postBuildScripts", std::vector<std::string>{});
        pImpl_->environmentVariables = pImpl_->configOptions->value(
            "environmentVariables", std::vector<std::string>{});
        pImpl_->dependencies = pImpl_->configOptions->value(
            "dependencies", std::vector<std::string>{});
    } catch (const json::exception& e) {
        pImpl_->logCallback("Failed to parse config file: " +
                            configPath.string() + " with " + e.what());
        return false;
    }

    return true;
}

auto CMakeBuilder::setLogCallback(
    std::function<void(const std::string&)> callback) -> void {
    pImpl_->logCallback = std::move(callback);
}

auto CMakeBuilder::getAvailableTargets(const std::filesystem::path& buildDir)
    -> std::vector<std::string> {
    std::string command =
        "cmake --build " + buildDir.string() + " --target help";
    auto [output, status] = atom::system::executeCommandWithStatus(command);

    std::vector<std::string> targets;
    if (status == 0 && !output.empty()) {
        std::istringstream iss(output);
        std::string line;
        std::regex targetRegex(R"(^\.\.\.\s+(.+))");
        while (std::getline(iss, line)) {
            std::smatch match;
            if (std::regex_search(line, match, targetRegex)) {
                targets.push_back(match[1]);
            }
        }
    }
    return targets;
}

auto CMakeBuilder::getCacheVariables(const std::filesystem::path& buildDir)
    -> std::vector<std::pair<std::string, std::string>> {
    std::string command = "cmake -LA -N " + buildDir.string();
    auto [output, status] = atom::system::executeCommandWithStatus(command);

    std::vector<std::pair<std::string, std::string>> variables;
    if (status == 0 && !output.empty()) {
        std::istringstream iss(output);
        std::string line;
        std::regex varRegex(R"(^([^:]+):([^=]+)=(.+)$)");
        while (std::getline(iss, line)) {
            std::smatch match;
            if (std::regex_search(line, match, varRegex)) {
                variables.emplace_back(match[1], match[3]);
            }
        }
    }
    return variables;
}

}  // namespace lithium
