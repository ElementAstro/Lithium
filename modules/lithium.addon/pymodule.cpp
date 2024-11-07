#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "addon/addons.hpp"
#include "addon/build_manager.hpp"
#include "addon/compile_command_generator.hpp"
#include "addon/compiler.hpp"
#include "addon/compiler_output_parser.hpp"
#include "addon/dependency.hpp"
#include "addon/generator.hpp"
#include "addon/loader.hpp"
#include "addon/manager.hpp"
#include "addon/sandbox.hpp"
#include "addon/system_dependency.hpp"
#include "addon/toolchain.hpp"
#include "addon/tracker.hpp"

namespace py = pybind11;
using namespace lithium;

PYBIND11_MODULE(lithium_bindings, m) {
    py::class_<AddonManager, std::shared_ptr<AddonManager>>(m, "AddonManager")
        .def(py::init<>())
        .def_static("createShared", &AddonManager::createShared)
        .def("addModule", &AddonManager::addModule)
        .def("removeModule", &AddonManager::removeModule)
        .def("getModule", &AddonManager::getModule)
        .def("resolveDependencies", &AddonManager::resolveDependencies);

    py::enum_<Project::BuildSystemType>(m, "BuildSystemType")
        .value("CMake", Project::BuildSystemType::CMake)
        .value("Meson", Project::BuildSystemType::Meson)
        .value("XMake", Project::BuildSystemType::XMake)
        .value("Unknown", Project::BuildSystemType::Unknown)
        .export_values();

    py::class_<Project>(m, "Project")
        .def(py::init<std::filesystem::path, std::filesystem::path,
                      Project::BuildSystemType>(),
             py::arg("sourceDir"), py::arg("buildDir"),
             py::arg("type") = Project::BuildSystemType::Unknown)
        .def("detectBuildSystem", &Project::detectBuildSystem)
        .def("getSourceDir", &Project::getSourceDir)
        .def("getBuildDir", &Project::getBuildDir)
        .def("getBuildSystemType", &Project::getBuildSystemType);

    py::class_<BuildManager>(m, "BuildManager")
        .def(py::init<>())
        .def("scanForProjects", &BuildManager::scanForProjects)
        .def("addProject", &BuildManager::addProject)
        .def("getProjects", &BuildManager::getProjects)
        .def("configureProject", &BuildManager::configureProject,
             py::arg("project"), py::arg("buildType"),
             py::arg("options") = std::vector<std::string>{},
             py::arg("envVars") = std::map<std::string, std::string>{})
        .def("buildProject", &BuildManager::buildProject, py::arg("project"),
             py::arg("jobs") = std::nullopt)
        .def("cleanProject", &BuildManager::cleanProject)
        .def("installProject", &BuildManager::installProject)
        .def("runTests", &BuildManager::runTests)
        .def("generateDocs", &BuildManager::generateDocs);

    py::class_<CompileCommandGenerator>(m, "CompileCommandGenerator")
        .def(py::init<>())
        .def("setOption", &CompileCommandGenerator::setOption,
             py::return_value_policy::reference)
        .def("addTarget", &CompileCommandGenerator::addTarget,
             py::return_value_policy::reference)
        .def("setTargetOption", &CompileCommandGenerator::setTargetOption,
             py::return_value_policy::reference)
        .def("addConditionalOption",
             &CompileCommandGenerator::addConditionalOption,
             py::return_value_policy::reference)
        .def("addDefine", &CompileCommandGenerator::addDefine,
             py::return_value_policy::reference)
        .def("addFlag", &CompileCommandGenerator::addFlag,
             py::return_value_policy::reference)
        .def("addLibrary", &CompileCommandGenerator::addLibrary,
             py::return_value_policy::reference)
        .def("setCommandTemplate", &CompileCommandGenerator::setCommandTemplate,
             py::return_value_policy::reference)
        .def("setCompiler", &CompileCommandGenerator::setCompiler,
             py::return_value_policy::reference)
        .def("loadConfigFromFile", &CompileCommandGenerator::loadConfigFromFile)
        .def("generate", &CompileCommandGenerator::generate);

    py::enum_<MessageType>(m, "MessageType")
        .value("ERROR", MessageType::ERROR)
        .value("WARNING", MessageType::WARNING)
        .value("NOTE", MessageType::NOTE)
        .value("UNKNOWN", MessageType::UNKNOWN)
        .export_values();

    py::class_<Message>(m, "Message")
        .def(py::init<MessageType, std::string, int, int, std::string,
                      std::string, std::string, std::string>(),
             py::arg("type"), py::arg("file"), py::arg("line"),
             py::arg("column"), py::arg("errorCode"), py::arg("functionName"),
             py::arg("message"), py::arg("context"))
        .def_readwrite("type", &Message::type)
        .def_readwrite("file", &Message::file)
        .def_readwrite("line", &Message::line)
        .def_readwrite("column", &Message::column)
        .def_readwrite("errorCode", &Message::errorCode)
        .def_readwrite("functionName", &Message::functionName)
        .def_readwrite("message", &Message::message)
        .def_readwrite("context", &Message::context)
        .def_readwrite("relatedNotes", &Message::relatedNotes);

    py::class_<CompilerOutputParser>(m, "CompilerOutputParser")
        .def(py::init<>())
        .def("parseLine", &CompilerOutputParser::parseLine)
        .def("parseFile", &CompilerOutputParser::parseFile)
        .def("parseFileMultiThreaded",
             &CompilerOutputParser::parseFileMultiThreaded)
        .def("getReport", &CompilerOutputParser::getReport,
             py::arg("detailed") = true)
        .def("generateHtmlReport", &CompilerOutputParser::generateHtmlReport)
        .def("generateJsonReport", &CompilerOutputParser::generateJsonReport)
        .def("setCustomRegexPattern",
             &CompilerOutputParser::setCustomRegexPattern);

    py::class_<Compiler>(m, "Compiler")
        .def(py::init<>())
        .def("compileToSharedLibrary", &Compiler::compileToSharedLibrary,
             py::arg("code"), py::arg("moduleName"), py::arg("functionName"),
             py::arg("optionsFile") = "compile_options.json")
        .def("addCompileOptions", &Compiler::addCompileOptions)
        .def("getAvailableCompilers", &Compiler::getAvailableCompilers)
        .def("generateCompileCommands", &Compiler::generateCompileCommands);

    py::class_<DependencyGraph>(m, "DependencyGraph")
        .def(py::init<>())
        .def("addNode", &DependencyGraph::addNode)
        .def("addDependency", &DependencyGraph::addDependency)
        .def("removeNode", &DependencyGraph::removeNode)
        .def("removeDependency", &DependencyGraph::removeDependency)
        .def("getDependencies", &DependencyGraph::getDependencies)
        .def("getDependents", &DependencyGraph::getDependents)
        .def("hasCycle", &DependencyGraph::hasCycle)
        .def("topologicalSort", &DependencyGraph::topologicalSort)
        .def("getAllDependencies", &DependencyGraph::getAllDependencies)
        .def("loadNodesInParallel", &DependencyGraph::loadNodesInParallel)
        .def("resolveDependencies", &DependencyGraph::resolveDependencies);

    py::class_<CppMemberGenerator>(m, "CppMemberGenerator")
        .def_static("generate", &CppMemberGenerator::generate<json>);

    py::class_<CppConstructorGenerator>(m, "CppConstructorGenerator")
        .def_static("generate", &CppConstructorGenerator::generate<json>);

    py::class_<CppDestructorGenerator>(m, "CppDestructorGenerator")
        .def_static("generate", &CppDestructorGenerator::generate<json>);

    py::class_<CppCopyMoveGenerator>(m, "CppCopyMoveGenerator")
        .def_static("generate", &CppCopyMoveGenerator::generate<json>);

    py::class_<CppMethodGenerator>(m, "CppMethodGenerator")
        .def_static("generate", &CppMethodGenerator::generate<json>);

    py::class_<CppAccessorGenerator>(m, "CppAccessorGenerator")
        .def_static("generate", &CppAccessorGenerator::generate<json>);

    py::class_<CppMutatorGenerator>(m, "CppMutatorGenerator")
        .def_static("generate", &CppMutatorGenerator::generate<json>);

    py::class_<CppFriendFunctionGenerator>(m, "CppFriendFunctionGenerator")
        .def_static("generate", &CppFriendFunctionGenerator::generate<json>);

    py::class_<CppFriendClassGenerator>(m, "CppFriendClassGenerator")
        .def_static("generate", &CppFriendClassGenerator::generate<json>);

    py::class_<CppOperatorOverloadGenerator>(m, "CppOperatorOverloadGenerator")
        .def_static("generate", &CppOperatorOverloadGenerator::generate<json>);

    py::class_<CppCodeGenerator>(m, "CppCodeGenerator")
        .def_static("generate", &CppCodeGenerator::generate<json>);

    py::class_<ModuleLoader, std::shared_ptr<ModuleLoader>>(m, "ModuleLoader")
        .def(py::init<std::string>())
        .def_static("createShared",
                    py::overload_cast<>(&ModuleLoader::createShared))
        .def_static("createShared",
                    py::overload_cast<std::string>(&ModuleLoader::createShared))
        .def("loadModule", &ModuleLoader::loadModule)
        .def("unloadModule", &ModuleLoader::unloadModule)
        .def("unloadAllModules", &ModuleLoader::unloadAllModules)
        .def("hasModule", &ModuleLoader::hasModule)
        .def("getModule", &ModuleLoader::getModule)
        .def("enableModule", &ModuleLoader::enableModule)
        .def("disableModule", &ModuleLoader::disableModule)
        .def("isModuleEnabled", &ModuleLoader::isModuleEnabled)
        .def("getAllExistedModules", &ModuleLoader::getAllExistedModules)
        .def("hasFunction", &ModuleLoader::hasFunction);

    py::class_<ComponentManager, std::shared_ptr<ComponentManager>>(
        m, "ComponentManager")
        .def(py::init<>())
        .def("initialize", &ComponentManager::initialize)
        .def("destroy", &ComponentManager::destroy)
        .def_static("createShared", &ComponentManager::createShared)
        .def("loadComponent", &ComponentManager::loadComponent)
        .def("unloadComponent", &ComponentManager::unloadComponent)
        .def("reloadComponent", &ComponentManager::reloadComponent)
        .def("reloadAllComponents", &ComponentManager::reloadAllComponents)
        .def("scanComponents", &ComponentManager::scanComponents)
        .def("getComponent", &ComponentManager::getComponent)
        .def("getComponentInfo", &ComponentManager::getComponentInfo)
        .def("getComponentList", &ComponentManager::getComponentList)
        .def("getComponentDoc", &ComponentManager::getComponentDoc)
        .def("hasComponent", &ComponentManager::hasComponent)
        .def("savePackageLock", &ComponentManager::savePackageLock)
        // .def("printDependencyTree", &ComponentManager::printDependencyTree)
        .def("compileAndLoadComponent",
             &ComponentManager::compileAndLoadComponent);

    py::class_<Sandbox>(m, "Sandbox")
        .def(py::init<>())
        .def("setTimeLimit", &Sandbox::setTimeLimit)
        .def("setMemoryLimit", &Sandbox::setMemoryLimit)
        .def("setRootDirectory", &Sandbox::setRootDirectory)
        .def("setUserId", &Sandbox::setUserId)
        .def("setProgramPath", &Sandbox::setProgramPath)
        .def("setProgramArgs", &Sandbox::setProgramArgs)
        .def("run", &Sandbox::run)
        .def("getTimeUsed", &Sandbox::getTimeUsed)
        .def("getMemoryUsed", &Sandbox::getMemoryUsed);

    py::class_<MultiSandbox>(m, "MultiSandbox")
        .def(py::init<>())
        .def("createSandbox", &MultiSandbox::createSandbox)
        .def("removeSandbox", &MultiSandbox::removeSandbox)
        .def("runAll", &MultiSandbox::runAll)
        .def("getSandboxTimeUsed", &MultiSandbox::getSandboxTimeUsed)
        .def("getSandboxMemoryUsed", &MultiSandbox::getSandboxMemoryUsed);

    py::enum_<LogLevel>(m, "LogLevel")
        .value("INFO", LogLevel::INFO)
        .value("WARNING", LogLevel::WARNING)
        .value("ERROR", LogLevel::ERROR)
        .export_values();

    py::class_<DependencyException>(m, "DependencyException")
        .def(py::init<std::string>())
        .def("what", &DependencyException::what);

    py::class_<DependencyInfo>(m, "DependencyInfo")
        .def(py::init<>())
        .def_readwrite("name", &DependencyInfo::name)
        .def_readwrite("version", &DependencyInfo::version);

    py::class_<DependencyManager>(m, "DependencyManager")
        .def(py::init<std::vector<DependencyInfo>>())
        .def("setLogCallback", &DependencyManager::setLogCallback)
        .def("checkAndInstallDependencies",
             &DependencyManager::checkAndInstallDependencies)
        .def("setCustomInstallCommand",
             &DependencyManager::setCustomInstallCommand)
        .def("generateDependencyReport",
             &DependencyManager::generateDependencyReport)
        .def("uninstallDependency", &DependencyManager::uninstallDependency)
        .def("getCurrentPlatform", &DependencyManager::getCurrentPlatform)
        .def("installDependencyAsync",
             &DependencyManager::installDependencyAsync)
        .def("cancelInstallation", &DependencyManager::cancelInstallation);

    py::enum_<Toolchain::Type>(m, "ToolchainType")
        .value("Compiler", Toolchain::Type::Compiler)
        .value("BuildTool", Toolchain::Type::BuildTool)
        .value("Unknown", Toolchain::Type::Unknown)
        .export_values();

    py::class_<Toolchain>(m, "Toolchain")
        .def(py::init<std::string, std::string, std::string, std::string,
                      std::string, Toolchain::Type>(),
             py::arg("name"), py::arg("compiler"), py::arg("buildTool"),
             py::arg("version"), py::arg("path"),
             py::arg("type") = Toolchain::Type::Unknown)
        .def("displayInfo", &Toolchain::displayInfo)
        .def("getName", &Toolchain::getName)
        .def("getCompiler", &Toolchain::getCompiler)
        .def("getBuildTool", &Toolchain::getBuildTool)
        .def("getVersion", &Toolchain::getVersion)
        .def("getPath", &Toolchain::getPath)
        .def("getType", &Toolchain::getType)
        .def("setVersion", &Toolchain::setVersion)
        .def("setPath", &Toolchain::setPath)
        .def("setType", &Toolchain::setType)
        .def("isCompatibleWith", &Toolchain::isCompatibleWith);

    py::class_<ToolchainManager>(m, "ToolchainManager")
        .def(py::init<>())
        .def("scanForToolchains", &ToolchainManager::scanForToolchains)
        .def("listToolchains", &ToolchainManager::listToolchains)
        .def("selectToolchain", &ToolchainManager::selectToolchain)
        .def("saveConfig", &ToolchainManager::saveConfig)
        .def("loadConfig", &ToolchainManager::loadConfig)
        .def("getToolchains", &ToolchainManager::getToolchains)
        .def("getAvailableCompilers", &ToolchainManager::getAvailableCompilers)
        .def("getAvailableBuildTools",
             &ToolchainManager::getAvailableBuildTools)
        .def("addToolchain", &ToolchainManager::addToolchain)
        .def("removeToolchain", &ToolchainManager::removeToolchain)
        .def("updateToolchain", &ToolchainManager::updateToolchain)
        .def("findToolchain", &ToolchainManager::findToolchain)
        .def("findToolchains", &ToolchainManager::findToolchains)
        .def("suggestCompatibleToolchains",
             &ToolchainManager::suggestCompatibleToolchains)
        .def("registerCustomToolchain",
             &ToolchainManager::registerCustomToolchain)
        .def("setDefaultToolchain", &ToolchainManager::setDefaultToolchain)
        .def("getDefaultToolchain", &ToolchainManager::getDefaultToolchain)
        .def("addSearchPath", &ToolchainManager::addSearchPath)
        .def("removeSearchPath", &ToolchainManager::removeSearchPath)
        .def("getSearchPaths", &ToolchainManager::getSearchPaths)
        .def("setToolchainAlias", &ToolchainManager::setToolchainAlias)
        .def("getToolchainByAlias", &ToolchainManager::getToolchainByAlias);

    py::class_<FileTracker>(m, "FileTracker")
        .def(py::init<std::string_view, std::string_view,
                      std::span<const std::string>, bool>(),
             py::arg("directory"), py::arg("jsonFilePath"),
             py::arg("fileTypes"), py::arg("recursive") = false)
        .def("scan", &FileTracker::scan)
        .def("compare", &FileTracker::compare)
        .def("logDifferences", &FileTracker::logDifferences)
        .def("recover", &FileTracker::recover)
        .def("asyncScan", &FileTracker::asyncScan)
        .def("asyncCompare", &FileTracker::asyncCompare)
        .def("getDifferences", &FileTracker::getDifferences)
        .def("getTrackedFileTypes", &FileTracker::getTrackedFileTypes)
        /*
        TODO: Implement this in the future
        .def("forEachFile",
             [](FileTracker& self, py::function func) {
                 self.forEachFile(
                     [&func](const fs::path& path) { func(path.string()); });
             })
        */
        .def("getFileInfo", &FileTracker::getFileInfo)
        .def("addFileType", &FileTracker::addFileType)
        .def("removeFileType", &FileTracker::removeFileType)
        .def("setEncryptionKey", &FileTracker::setEncryptionKey);
}
