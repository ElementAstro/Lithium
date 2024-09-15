// toolchain.cpp
#include "toolchain.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "atom/log/loguru.hpp"
#include "error/exception.hpp"
#include "utils/constant.hpp"

// Toolchain implementation
class Toolchain::Impl {
public:
    std::string name;
    std::string compiler;
    std::string buildTool;
    std::string version;
    std::string path;
    Type type;

    Impl(std::string name, std::string compiler, std::string buildTool,
         std::string version, std::string path, Type type)
        : name(std::move(name)),
          compiler(std::move(compiler)),
          buildTool(std::move(buildTool)),
          version(std::move(version)),
          path(std::move(path)),
          type(type) {}
};

Toolchain::Toolchain(std::string name, std::string compiler,
                     std::string buildTool, std::string version,
                     std::string path, Type type)
    : impl_(std::make_unique<Impl>(std::move(name), std::move(compiler),
                                   std::move(buildTool), std::move(version),
                                   std::move(path), type)) {}

Toolchain::~Toolchain() = default;
Toolchain::Toolchain(const Toolchain& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {}
Toolchain::Toolchain(Toolchain&& other) noexcept = default;
auto Toolchain::operator=(const Toolchain& other) -> Toolchain& {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}
auto Toolchain::operator=(Toolchain&& other) noexcept -> Toolchain& = default;

void Toolchain::displayInfo() const {
    LOG_F(INFO, "Toolchain Information for {}", impl_->name);
    LOG_F(INFO, "Compiler: {}", impl_->compiler);
    LOG_F(INFO, "Build Tool: {}", impl_->buildTool);
    LOG_F(INFO, "Version: {}", impl_->version);
    LOG_F(INFO, "Path: {}", impl_->path);
    LOG_F(INFO, "Type: {}",
          impl_->type == Type::Compiler
              ? "Compiler"
              : (impl_->type == Type::BuildTool ? "Build Tool" : "Unknown"));
}

auto Toolchain::getName() const -> const std::string& { return impl_->name; }
auto Toolchain::getCompiler() const -> const std::string& {
    return impl_->compiler;
}
auto Toolchain::getBuildTool() const -> const std::string& {
    return impl_->buildTool;
}
auto Toolchain::getVersion() const -> const std::string& {
    return impl_->version;
}
auto Toolchain::getPath() const -> const std::string& { return impl_->path; }
auto Toolchain::getType() const -> Type { return impl_->type; }

void Toolchain::setVersion(const std::string& version) {
    impl_->version = version;
}
void Toolchain::setPath(const std::string& path) { impl_->path = path; }
void Toolchain::setType(Type type) { impl_->type = type; }

auto Toolchain::isCompatibleWith(const Toolchain& other) const -> bool {
    // Implement compatibility logic here
    // For example, check if compiler versions are compatible
    return true;  // Placeholder implementation
}

// ToolchainManager implementation
class ToolchainManager::Impl {
public:
    std::vector<Toolchain> toolchains;
    std::vector<std::string> searchPaths;
    std::unordered_map<std::string, std::string> toolchainAliases;
    std::optional<std::string> defaultToolchain;

    static auto getCompilerVersion(const std::string& path) -> std::string;
    void scanBuildTools();
    static auto executeCommand(const std::string& command) -> std::string;
    void initializeDefaultSearchPaths();
};

ToolchainManager::ToolchainManager() : impl_(std::make_unique<Impl>()) {
    impl_->initializeDefaultSearchPaths();
}

ToolchainManager::~ToolchainManager() = default;
ToolchainManager::ToolchainManager(ToolchainManager&&) noexcept = default;
auto ToolchainManager::operator=(ToolchainManager&&) noexcept
    -> ToolchainManager& = default;

void ToolchainManager::scanForToolchains() {
    for (const auto& path : impl_->searchPaths) {
        if (std::filesystem::exists(path)) {
            for (const auto& entry :
                 std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.starts_with("gcc") ||
                        filename.starts_with("g++") ||
                        filename.starts_with("clang") ||
                        filename.starts_with("clang++")) {
                        std::string version =
                            Impl::getCompilerVersion(entry.path().string());
                        addToolchain(Toolchain(filename, filename, "", version,
                                               entry.path().string(),
                                               Toolchain::Type::Compiler));
                    }
                }
            }
        }
    }

    impl_->scanBuildTools();
}

void ToolchainManager::Impl::scanBuildTools() {
    std::vector<std::string> buildTools = {"make", "ninja", "cmake", "gmake",
                                           "msbuild"};

    for (const auto& tool : buildTools) {
        std::string toolPath = tool + Constants::EXECUTABLE_EXTENSION;
        if (std::filesystem::exists(toolPath)) {
            std::string version = getCompilerVersion(toolPath);
            toolchains.emplace_back(tool, "", tool, version, toolPath,
                                    Toolchain::Type::BuildTool);
        }
    }
}

void ToolchainManager::listToolchains() const {
    LOG_F(INFO, "Available Toolchains:");
    for (const auto& tc : impl_->toolchains) {
        LOG_F(INFO, "- {} ({}) [{}]", tc.getName(), tc.getVersion(),
              tc.getType() == Toolchain::Type::Compiler
                  ? "Compiler"
                  : (tc.getType() == Toolchain::Type::BuildTool ? "Build Tool"
                                                                : "Unknown"));
    }
}

auto ToolchainManager::selectToolchain(const std::string& name) const
    -> std::optional<Toolchain> {
    auto it = std::find_if(
        impl_->toolchains.begin(), impl_->toolchains.end(),
        [&name](const Toolchain& tc) { return tc.getName() == name; });
    if (it != impl_->toolchains.end()) {
        it->displayInfo();
        return *it;
    }
    return std::nullopt;
}

void ToolchainManager::saveConfig(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        THROW_FAIL_TO_CLOSE_FILE("Unable to open file for writing: " +
                                 filename);
    }
    for (const auto& tc : impl_->toolchains) {
        file << tc.getName() << "," << tc.getCompiler() << ","
             << tc.getBuildTool() << "," << tc.getVersion() << ","
             << tc.getPath() << "," << static_cast<int>(tc.getType()) << "\n";
    }
    // Save aliases
    file << "--- Aliases ---\n";
    for (const auto& [alias, toolchainName] : impl_->toolchainAliases) {
        file << alias << "," << toolchainName << "\n";
    }
    // Save default toolchain
    file << "--- Default ---\n";
    if (impl_->defaultToolchain) {
        file << *impl_->defaultToolchain << "\n";
    }
    LOG_F(INFO, "Configuration saved to {}", filename);
}

void ToolchainManager::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading: " +
                                 filename);
    }
    impl_->toolchains.clear();
    impl_->toolchainAliases.clear();
    impl_->defaultToolchain.reset();

    std::string line;
    bool readingAliases = false;
    bool readingDefault = false;
    while (std::getline(file, line)) {
        if (line == "--- Aliases ---") {
            readingAliases = true;
            readingDefault = false;
            continue;
        } else if (line == "--- Default ---") {
            readingAliases = false;
            readingDefault = true;
            continue;
        }

        if (readingAliases) {
            std::istringstream iss(line);
            std::string alias;
            std::string toolchainName;
            if (std::getline(iss, alias, ',') &&
                std::getline(iss, toolchainName)) {
                setToolchainAlias(alias, toolchainName);
            }
        } else if (readingDefault) {
            setDefaultToolchain(line);
        } else {
            std::vector<std::string> parts;
            std::istringstream iss(line);
            std::string part;
            while (std::getline(iss, part, ',')) {
                parts.push_back(part);
            }
            if (parts.size() == 6) {
                addToolchain(Toolchain(
                    parts[0], parts[1], parts[2], parts[3], parts[4],
                    static_cast<Toolchain::Type>(std::stoi(parts[5]))));
            }
        }
    }
    LOG_F(INFO, "Configuration loaded from {}", filename);
}

auto ToolchainManager::getToolchains() const -> const std::vector<Toolchain>& {
    return impl_->toolchains;
}

auto ToolchainManager::getAvailableCompilers() const
    -> std::vector<std::string> {
    std::vector<std::string> compilers;
    for (const auto& tc : impl_->toolchains) {
        if (tc.getType() == Toolchain::Type::Compiler) {
            compilers.push_back(tc.getName());
        }
    }
    return compilers;
}

auto ToolchainManager::getAvailableBuildTools() const
    -> std::vector<std::string> {
    std::vector<std::string> buildTools;
    for (const auto& tc : impl_->toolchains) {
        if (tc.getType() == Toolchain::Type::BuildTool) {
            buildTools.push_back(tc.getName());
        }
    }
    return buildTools;
}

void ToolchainManager::addToolchain(const Toolchain& toolchain) {
    auto it = std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                           [&](const Toolchain& tc) {
                               return tc.getName() == toolchain.getName();
                           });
    if (it == impl_->toolchains.end()) {
        impl_->toolchains.push_back(toolchain);
    } else {
        *it = toolchain;
    }
}

void ToolchainManager::removeToolchain(const std::string& name) {
    impl_->toolchains.erase(
        std::remove_if(
            impl_->toolchains.begin(), impl_->toolchains.end(),
            [&](const Toolchain& tc) { return tc.getName() == name; }),
        impl_->toolchains.end());
}

void ToolchainManager::updateToolchain(const std::string& name,
                                       const Toolchain& updatedToolchain) {
    auto it =
        std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                     [&](const Toolchain& tc) { return tc.getName() == name; });
    if (it != impl_->toolchains.end()) {
        *it = updatedToolchain;
    }
}

auto ToolchainManager::findToolchain(const std::string& name) const
    -> std::optional<Toolchain> {
    auto it =
        std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                     [&](const Toolchain& tc) { return tc.getName() == name; });
    if (it != impl_->toolchains.end()) {
        return *it;
    }
    return std::nullopt;
}

auto ToolchainManager::findToolchains(const ToolchainFilter& filter) const
    -> std::vector<Toolchain> {
    std::vector<Toolchain> result;
    std::copy_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                 std::back_inserter(result), filter);
    return result;
}

auto ToolchainManager::suggestCompatibleToolchains(const Toolchain& base) const
    -> std::vector<Toolchain> {
    return findToolchains(
        [&base](const Toolchain& tc) { return base.isCompatibleWith(tc); });
}

void ToolchainManager::registerCustomToolchain(const std::string& name,
                                               const std::string& path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Custom toolchain path does not exist: " +
                                 path);
    }
    std::string version = Impl::getCompilerVersion(path);
    Toolchain::Type type = path.find("make") != std::string::npos ||
                                   path.find("ninja") != std::string::npos
                               ? Toolchain::Type::BuildTool
                               : Toolchain::Type::Compiler;
    addToolchain(Toolchain(name, name, "", version, path, type));
}

void ToolchainManager::setDefaultToolchain(const std::string& name) {
    if (findToolchain(name)) {
        impl_->defaultToolchain = name;
    } else {
        throw std::runtime_error("Toolchain not found: " + name);
    }
}

auto ToolchainManager::getDefaultToolchain() const -> std::optional<Toolchain> {
    if (impl_->defaultToolchain) {
        return findToolchain(*impl_->defaultToolchain);
    }
    return std::nullopt;
}

void ToolchainManager::addSearchPath(const std::string& path) {
    if (std::find(impl_->searchPaths.begin(), impl_->searchPaths.end(), path) ==
        impl_->searchPaths.end()) {
        impl_->searchPaths.push_back(path);
    }
}

void ToolchainManager::removeSearchPath(const std::string& path) {
    impl_->searchPaths.erase(
        std::remove(impl_->searchPaths.begin(), impl_->searchPaths.end(), path),
        impl_->searchPaths.end());
}

auto ToolchainManager::getSearchPaths() const
    -> const std::vector<std::string>& {
    return impl_->searchPaths;
}

void ToolchainManager::setToolchainAlias(const std::string& alias,
                                         const std::string& toolchainName) {
    if (findToolchain(toolchainName)) {
        impl_->toolchainAliases[alias] = toolchainName;
    } else {
        throw std::runtime_error("Toolchain not found: " + toolchainName);
    }
}

auto ToolchainManager::getToolchainByAlias(const std::string& alias) const
    -> std::optional<Toolchain> {
    auto it = impl_->toolchainAliases.find(alias);
    if (it != impl_->toolchainAliases.end()) {
        return findToolchain(it->second);
    }
    return std::nullopt;
}

auto ToolchainManager::Impl::getCompilerVersion(const std::string& path)
    -> std::string {
    std::string command = "\"" + path + "\" --version";
    std::string result = executeCommand(command);
    return result.empty() ? "Unknown version"
                          : result.substr(0, result.find('\n'));
}

auto ToolchainManager::Impl::executeCommand(const std::string& command)
    -> std::string {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void ToolchainManager::Impl::initializeDefaultSearchPaths() {
#if defined(_WIN32) || defined(_WIN64)
    searchPaths = {"C:\\Program Files",        "C:\\Program Files (x86)",
                   "C:\\MinGW\\bin",           "C:\\LLVM\\bin",
                   "C:\\msys64\\mingw64\\bin", "C:\\msys64\\mingw32\\bin",
                   "C:\\msys64\\clang64\\bin", "C:\\msys64\\clang32\\bin"};
#else
    searchPaths = {"/usr/bin", "/usr/local/bin", "/opt/local/bin"};
#endif
}
