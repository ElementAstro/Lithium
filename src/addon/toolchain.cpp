#include "toolchain.hpp"

#include "config.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/memory/utils.hpp"
#include "atom/system/command.hpp"
#include "atom/system/env.hpp"
#include "atom/utils/to_string.hpp"

#if ENABLE_ASYNC
#include "atom/io/async_io.hpp"
#else
#include "atom/io/io.hpp"
#endif

#include "utils/constant.hpp"

// 重载输出操作符
template <typename T,
          typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
std::ostream& operator<<(std::ostream& os, const T& value) {
    return os << static_cast<int>(value);
}

// 工具链实现
class Toolchain::Impl {
public:
    std::string name;
    std::string compiler;
    std::string buildTool;
    std::string version;
    std::string path;
    Toolchain::ToolchainType type;

    Impl(std::string name, std::string compiler, std::string buildTool,
         std::string version, std::string path, Toolchain::ToolchainType type)
        : name(std::move(name)),
          compiler(std::move(compiler)),
          buildTool(std::move(buildTool)),
          version(std::move(version)),
          path(std::move(path)),
          type(type) {}
};

Toolchain::Toolchain(std::string name, std::string compiler,
                     std::string buildTool, std::string version,
                     std::string path, Toolchain::ToolchainType type)
    : impl_(std::make_unique<Impl>(std::move(name), std::move(compiler),
                                   std::move(buildTool), std::move(version),
                                   std::move(path), type)) {
    LOG_F(INFO, "Created Toolchain: {}", impl_->name);
}

Toolchain::~Toolchain() { LOG_F(INFO, "Destroyed Toolchain: {}", impl_->name); }

Toolchain::Toolchain(const Toolchain& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {
    LOG_F(INFO, "Copied Toolchain: {}", impl_->name);
}

Toolchain::Toolchain(Toolchain&& other) noexcept = default;

Toolchain& Toolchain::operator=(const Toolchain& other) {
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
        LOG_F(INFO, "Assigned Toolchain: {}", impl_->name);
    }
    return *this;
}

Toolchain& Toolchain::operator=(Toolchain&& other) noexcept = default;

void Toolchain::displayInfo() const {
    LOG_F(INFO, "Displaying Toolchain info: {}", impl_->name);
    LOG_F(INFO, "Compiler: {}", impl_->compiler);
    LOG_F(INFO, "Build Tool: {}", impl_->buildTool);
    LOG_F(INFO, "Version: {}", impl_->version);
    LOG_F(INFO, "Path: {}", impl_->path);
    LOG_F(
        INFO, "Type: {}",
        impl_->type == Toolchain::ToolchainType::Compiler
            ? "Compiler"
            : (impl_->type == Toolchain::ToolchainType::BuildTool ? "Build Tool"
                                                                  : "Unknown"));
}

const std::string& Toolchain::getName() const {
    LOG_F(INFO, "Getting Toolchain name: {}", impl_->name);
    return impl_->name;
}

const std::string& Toolchain::getCompiler() const {
    LOG_F(INFO, "Getting compiler: {}", impl_->compiler);
    return impl_->compiler;
}

const std::string& Toolchain::getBuildTool() const {
    LOG_F(INFO, "Getting build tool: {}", impl_->buildTool);
    return impl_->buildTool;
}

const std::string& Toolchain::getVersion() const {
    LOG_F(INFO, "Getting version: {}", impl_->version);
    return impl_->version;
}

const std::string& Toolchain::getPath() const {
    LOG_F(INFO, "Getting path: {}", impl_->path);
    return impl_->path;
}

Toolchain::ToolchainType Toolchain::getType() const {
    LOG_F(INFO, "Getting type: {}",
          (impl_->type == Toolchain::ToolchainType::Compiler
               ? "Compiler"
               : (impl_->type == Toolchain::ToolchainType::BuildTool
                      ? "Build Tool"
                      : "Unknown")));
    return impl_->type;
}

void Toolchain::setVersion(const std::string& version) {
    if (version.empty()) {
        LOG_F(ERROR, "Version cannot be empty.");
        throw std::invalid_argument("Version cannot be empty.");
    }
    LOG_F(INFO, "Setting version: {} -> {}", impl_->version, version);
    impl_->version = version;
}

void Toolchain::setPath(const std::string& path) {
    if (path.empty()) {
        LOG_F(ERROR, "Path cannot be empty.");
        throw std::invalid_argument("Path cannot be empty.");
    }
    LOG_F(INFO, "Setting path: {} -> {}", impl_->path, path);
    impl_->path = path;
}

void Toolchain::setType(Toolchain::ToolchainType type) {
    LOG_F(INFO, "Setting type: {} -> {}", static_cast<int>(impl_->type),
          type == Toolchain::ToolchainType::Compiler
              ? "Compiler"
              : (type == Toolchain::ToolchainType::BuildTool ? "Build Tool"
                                                             : "Unknown"));
    impl_->type = type;
}

bool Toolchain::isCompatibleWith(
    const std::shared_ptr<Toolchain>& other) const {
    if (!other) {
        LOG_F(ERROR, "Other toolchain is null.");
        return false;
    }
    LOG_F(INFO, "Checking compatibility with {}", other->getName());
    bool compatible = (impl_->compiler == other->impl_->compiler);
    LOG_F(INFO, "Compatibility result: {}",
          compatible ? "Compatible" : "Incompatible");
    return compatible;
}

// ToolchainManager实现
class ToolchainManager::Impl {
public:
    std::vector<std::shared_ptr<Toolchain>> toolchains;
    std::vector<std::string> searchPaths;
    std::unordered_map<std::string, std::string> toolchainAliases;
    std::optional<std::string> defaultToolchain;

    static std::string getCompilerVersion(const std::string& path);
    void scanBuildTools();
    void initializeDefaultSearchPaths();
    void loadToolchainsFromEnvironment();
};

ToolchainManager::ToolchainManager() : impl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "Initializing ToolchainManager");
    impl_->initializeDefaultSearchPaths();
    impl_->loadToolchainsFromEnvironment();
}

ToolchainManager::~ToolchainManager() {
    LOG_F(INFO, "Destroying ToolchainManager");
}

ToolchainManager::ToolchainManager(ToolchainManager&&) noexcept = default;

ToolchainManager& ToolchainManager::operator=(ToolchainManager&&) noexcept =
    default;

void ToolchainManager::scanForToolchains() {
    LOG_F(INFO, "Scanning for toolchains");
    for (const auto& path : impl_->searchPaths) {
        LOG_F(INFO, "Searching path: {}", path);
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
                        LOG_F(INFO, "Found compiler: {} version: {}", filename,
                              version);
                        auto toolchain = std::make_shared<Toolchain>(
                            filename, filename, filename, version,
                            entry.path().string(),
                            Toolchain::ToolchainType::Compiler);
                        addToolchain(*toolchain);
                    }
                }
            }
        } else {
            LOG_F(WARNING, "Path does not exist: {}", path);
        }
    }

    impl_->scanBuildTools();
}

void ToolchainManager::Impl::scanBuildTools() {
    LOG_F(INFO, "Scanning for build tools");
    std::vector<std::string> buildTools = {"make", "ninja", "cmake", "gmake",
                                           "msbuild"};

    for (const auto& tool : buildTools) {
        std::string toolPath = tool + Constants::EXECUTABLE_EXTENSION;
        if (std::filesystem::exists(toolPath)) {
            std::string version = getCompilerVersion(toolPath);
            LOG_F(INFO, "Found build tool: {} version: {}", tool, version);
            auto toolchain = std::make_shared<Toolchain>(
                tool, "", tool, version, toolPath,
                Toolchain::ToolchainType::BuildTool);
            toolchains.emplace_back(toolchain);
        } else {
            LOG_F(INFO, "Build tool not found: {}", tool);
        }
    }
}

void ToolchainManager::listToolchains() const {
    LOG_F(INFO, "Listing available toolchains");
    for (const auto& stc : impl_->toolchains) {
        if (stc) {
            auto tc = *stc;
            LOG_F(INFO, "- {} ({}) [{}]", tc.getName(), tc.getVersion(),
                  tc.getType() == Toolchain::ToolchainType::Compiler
                      ? "Compiler"
                      : (tc.getType() == Toolchain::ToolchainType::BuildTool
                             ? "Build Tool"
                             : "Unknown"));
        }
    }
}

std::optional<std::shared_ptr<Toolchain>> ToolchainManager::selectToolchain(
    const std::string& name) const {
    LOG_F(INFO, "Selecting toolchain: {}", name);
    auto it = std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                           [&name](const std::shared_ptr<Toolchain>& tc) {
                               return tc && tc->getName() == name;
                           });
    if (it != impl_->toolchains.end() && *it) {
        (*it)->displayInfo();
        return *it;
    }
    LOG_F(ERROR, "Toolchain not found: {}", name);
    return std::nullopt;
}

void ToolchainManager::saveConfig(const std::string& filename) const {
    LOG_F(INFO, "Saving config to file: {}", filename);
    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Unable to open file for writing: {}", filename);
        THROW_FAIL_TO_CLOSE_FILE("Unable to open file for writing: " +
                                 filename);
    }
    for (const auto& stc : impl_->toolchains) {
        if (stc) {
            auto tc = *stc;
            file << tc.getName() << "," << tc.getCompiler() << ","
                 << tc.getBuildTool() << "," << tc.getVersion() << ","
                 << tc.getPath() << "," << static_cast<int>(tc.getType())
                 << "\n";
        }
    }
    // 保存别名
    file << "--- Aliases ---\n";
    for (const auto& [alias, toolchainName] : impl_->toolchainAliases) {
        file << alias << "," << toolchainName << "\n";
    }
    // 保存默认工具链
    file << "--- Default ---\n";
    if (impl_->defaultToolchain) {
        file << *impl_->defaultToolchain << "\n";
    }
    LOG_F(INFO, "Config saved to {}", filename);
}

void ToolchainManager::loadConfig(const std::string& filename) {
    LOG_F(INFO, "Loading config file: {}", filename);
    std::ifstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Unable to open file for reading: {}", filename);
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
                parts.emplace_back(part);
            }
            if (parts.size() == 6) {
                Toolchain::ToolchainType type =
                    Toolchain::ToolchainType::Unknown;
                try {
                    type = static_cast<Toolchain::ToolchainType>(
                        std::stoi(parts[5]));
                } catch (...) {
                    LOG_F(WARNING, "Invalid toolchain type for {}", parts[0]);
                }
                addToolchain(Toolchain(parts[0], parts[1], parts[2], parts[3],
                                       parts[4], type));
            }
        }
    }
    LOG_F(INFO, "Config loaded from {}", filename);
}

const std::vector<std::shared_ptr<Toolchain>>& ToolchainManager::getToolchains()
    const {
    LOG_F(INFO, "Getting all toolchains");
    return impl_->toolchains;
}

std::vector<std::string> ToolchainManager::getAvailableCompilers() const {
    LOG_F(INFO, "Getting available compilers");
    std::vector<std::string> compilers;
    for (const auto& stc : impl_->toolchains) {
        if (stc && stc->getType() == Toolchain::ToolchainType::Compiler) {
            compilers.emplace_back(stc->getName());
        }
    }
    return compilers;
}

std::vector<std::string> ToolchainManager::getAvailableBuildTools() const {
    LOG_F(INFO, "Getting available build tools");
    std::vector<std::string> buildTools;
    for (const auto& stc : impl_->toolchains) {
        if (stc && stc->getType() == Toolchain::ToolchainType::BuildTool) {
            buildTools.emplace_back(stc->getName());
        }
    }
    return buildTools;
}

void ToolchainManager::addToolchain(const Toolchain& toolchain) {
    if (toolchain.getName().empty()) {
        LOG_F(ERROR, "Toolchain name cannot be empty.");
        throw std::invalid_argument("Toolchain name cannot be empty.");
    }
    LOG_F(INFO, "Adding toolchain: {}", toolchain.getName());
    auto it =
        std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                     [&](const std::shared_ptr<Toolchain>& tc) {
                         return tc && tc->getName() == toolchain.getName();
                     });
    if (it == impl_->toolchains.end()) {
        impl_->toolchains.emplace_back(std::make_shared<Toolchain>(toolchain));
    } else {
        *it = std::make_shared<Toolchain>(toolchain);
        LOG_F(INFO, "Toolchain already exists, updated: {}",
              toolchain.getName());
    }
}

void ToolchainManager::removeToolchain(const std::string& name) {
    LOG_F(INFO, "Removing toolchain: {}", name);
    impl_->toolchains.erase(
        std::remove_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                       [&](const std::shared_ptr<Toolchain>& tc) {
                           return tc && tc->getName() == name;
                       }),
        impl_->toolchains.end());
}

void ToolchainManager::updateToolchain(const std::string& name,
                                       const Toolchain& updatedToolchain) {
    if (updatedToolchain.getName() != name) {
        LOG_F(ERROR, "Toolchain name mismatch.");
        throw std::invalid_argument("Toolchain name mismatch.");
    }
    LOG_F(INFO, "Updating toolchain: {}", name);
    auto it = std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                           [&](const std::shared_ptr<Toolchain>& tc) {
                               return tc && tc->getName() == name;
                           });
    if (it != impl_->toolchains.end() && *it) {
        *it = std::make_shared<Toolchain>(updatedToolchain);
        LOG_F(INFO, "Toolchain updated: {}", name);
    } else {
        LOG_F(ERROR, "Toolchain not found: {}", name);
    }
}

std::optional<std::shared_ptr<Toolchain>> ToolchainManager::findToolchain(
    const std::string& name) const {
    LOG_F(INFO, "Finding toolchain: {}", name);
    auto it = std::find_if(impl_->toolchains.begin(), impl_->toolchains.end(),
                           [&](const std::shared_ptr<Toolchain>& tc) {
                               return tc && tc->getName() == name;
                           });
    if (it != impl_->toolchains.end()) {
        return *it;
    }
    LOG_F(WARNING, "Toolchain not found: {}", name);
    return std::nullopt;
}

std::vector<std::shared_ptr<Toolchain>> ToolchainManager::findToolchains(
    const ToolchainFilter& filter) const {
    LOG_F(INFO, "Finding toolchains with filter");
    std::vector<std::shared_ptr<Toolchain>> result;
    for (const auto& tc : impl_->toolchains) {
        if (tc && filter(tc)) {
            result.emplace_back(tc);
        }
    }
    return result;
}

std::vector<std::shared_ptr<Toolchain>>
ToolchainManager::suggestCompatibleToolchains(const Toolchain& base) const {
    LOG_F(INFO, "Suggesting compatible toolchains for {}", base.getName());
    return findToolchains([&base](const std::shared_ptr<Toolchain>& tc) {
        return tc && base.isCompatibleWith(tc);
    });
}

void ToolchainManager::registerCustomToolchain(const std::string& name,
                                               const std::string& path) {
    if (name.empty() || path.empty()) {
        LOG_F(ERROR, "Name and path cannot be empty.");
        throw std::invalid_argument("Name and path cannot be empty.");
    }
    LOG_F(INFO, "Registering custom toolchain: {} path: {}", name, path);
#if ENABLE_ASYNC
    std::weak_ptr<asio::io_context> ioContext;
    GET_OR_CREATE_WEAK_PTR(ioContext, asio::io_context, Constants::ASYNC_IO);
    if (auto ioContextPtr = ioContext.lock()) {
        atom::async::io::AsyncDirectory asyncDir(*ioContextPtr);
        asyncDir.asyncExists(path, [this, name, path](bool exists) {
            if (exists) {
                std::string version = Impl::getCompilerVersion(path);
                Toolchain::ToolchainType type =
                    (path.find("make") != std::string::npos ||
                     path.find("ninja") != std::string::npos)
                        ? Toolchain::ToolchainType::BuildTool
                        : Toolchain::ToolchainType::Compiler;
                addToolchain(Toolchain(name, name, "", version, path, type));
            } else {
                LOG_F(ERROR, "Custom toolchain path does not exist: {}", path);
                THROW_NOT_FOUND("Custom toolchain path does not exist: " +
                                path);
            }
        });
    } else {
        LOG_F(ERROR, "Failed to lock ioContext");
        THROW_OBJ_NOT_EXIST("ioContext");
    }
#else
    if (!atom::io::isFolderExists(path)) {
        LOG_F(ERROR, "Custom toolchain path does not exist: {}", path);
        THROW_NOT_FOUND("Custom toolchain path does not exist: " + path);
    }
    std::string version = Impl::getCompilerVersion(path);
    Toolchain::ToolchainType type = (path.find("make") != std::string::npos ||
                                     path.find("ninja") != std::string::npos)
                                        ? Toolchain::ToolchainType::BuildTool
                                        : Toolchain::ToolchainType::Compiler;
    addToolchain(Toolchain(name, name, "", version, path, type));
#endif
}

void ToolchainManager::setDefaultToolchain(const std::string& name) {
    LOG_F(INFO, "Setting default toolchain: {}", name);
    if (findToolchain(name)) {
        impl_->defaultToolchain = name;
    } else {
        LOG_F(ERROR, "Toolchain not found: {}", name);
        THROW_NOT_FOUND("Toolchain not found: " + name);
    }
}

std::optional<std::shared_ptr<Toolchain>>
ToolchainManager::getDefaultToolchain() const {
    LOG_F(INFO, "Getting default toolchain");
    if (impl_->defaultToolchain) {
        return findToolchain(*impl_->defaultToolchain);
    }
    return std::nullopt;
}

void ToolchainManager::addSearchPath(const std::string& path) {
    if (path.empty()) {
        LOG_F(ERROR, "Search path cannot be empty.");
        throw std::invalid_argument("Search path cannot be empty.");
    }
    LOG_F(INFO, "Adding search path: {}", path);
    if (std::find(impl_->searchPaths.begin(), impl_->searchPaths.end(), path) ==
        impl_->searchPaths.end()) {
        impl_->searchPaths.emplace_back(path);
    }
}

void ToolchainManager::removeSearchPath(const std::string& path) {
    LOG_F(INFO, "Removing search path: {}", path);
    impl_->searchPaths.erase(
        std::remove(impl_->searchPaths.begin(), impl_->searchPaths.end(), path),
        impl_->searchPaths.end());
}

const std::vector<std::string>& ToolchainManager::getSearchPaths() const {
    LOG_F(INFO, "Getting search paths");
    return impl_->searchPaths;
}

void ToolchainManager::setToolchainAlias(const std::string& alias,
                                         const std::string& toolchainName) {
    if (alias.empty() || toolchainName.empty()) {
        LOG_F(ERROR, "Alias and toolchain name cannot be empty.");
        throw std::invalid_argument(
            "Alias and toolchain name cannot be empty.");
    }
    LOG_F(INFO, "Setting toolchain alias: {} -> {}", alias, toolchainName);
    if (findToolchain(toolchainName)) {
        impl_->toolchainAliases[alias] = toolchainName;
    } else {
        LOG_F(ERROR, "Toolchain not found: {}", toolchainName);
        throw std::runtime_error("Toolchain not found: " + toolchainName);
    }
}

std::optional<std::shared_ptr<Toolchain>> ToolchainManager::getToolchainByAlias(
    const std::string& alias) const {
    LOG_F(INFO, "Getting toolchain by alias: {}", alias);
    auto it = impl_->toolchainAliases.find(alias);
    if (it != impl_->toolchainAliases.end()) {
        return findToolchain(it->second);
    }
    LOG_F(WARNING, "Toolchain alias not found: {}", alias);
    return std::nullopt;
}

std::string ToolchainManager::Impl::getCompilerVersion(
    const std::string& path) {
    LOG_F(INFO, "Getting compiler version, path: {}", path);
    std::string command = "\"" + path + "\" --version";
    std::string result = atom::system::executeCommand(command);
    if (result.empty()) {
        LOG_F(WARNING, "Unable to get version information");
        return "Unknown version";
    }
    std::string versionLine = result.substr(0, result.find('\n'));
    LOG_F(INFO, "Version information: {}", versionLine);
    return versionLine;
}

void ToolchainManager::Impl::initializeDefaultSearchPaths() {
#if defined(_WIN32) || defined(_WIN64)
    LOG_F(INFO, "Initializing default search paths (Windows)");
    searchPaths = {"C:\\Program Files",        "C:\\Program Files (x86)",
                   "C:\\MinGW\\bin",           "C:\\LLVM\\bin",
                   "C:\\msys64\\mingw64\\bin", "C:\\msys64\\mingw32\\bin",
                   "C:\\msys64\\clang64\\bin", "C:\\msys64\\clang32\\bin",
                   "C:\\msys64\\ucrt64\\bin",  "C:\\msys64\\msys2\\bin"};
#else
    LOG_F(INFO, "Initializing default search paths (Linux)");
    searchPaths = {"/usr/bin", "/usr/local/bin", "/opt/local/bin"};
    LOG_F(INFO, "Search paths: {}", atom::utils::toString(searchPaths));
#endif
}

void ToolchainManager::Impl::loadToolchainsFromEnvironment() {
    LOG_F(INFO, "Loading toolchain paths from environment variables");
    std::weak_ptr<atom::utils::Env> envPtr;
    GET_OR_CREATE_WEAK_PTR(envPtr, atom::utils::Env, Constants::ENVIRONMENT);
    auto env = envPtr.lock();
    if (env) {
        if (auto envP = env->get("TOOLCHAIN_PATHS"); !envP.empty()) {
            std::istringstream envPaths(envP);
            std::string path;
            while (std::getline(envPaths, path, ':')) {
                if (!path.empty()) {
                    searchPaths.emplace_back(path);
                    LOG_F(INFO,
                          "Added search path from environment variable: {}",
                          path);
                }
            }
        } else {
            LOG_F(INFO, "Environment variable TOOLCHAIN_PATHS not set");
        }
    } else {
        LOG_F(WARNING, "Environment pointer is expired.");
    }
}