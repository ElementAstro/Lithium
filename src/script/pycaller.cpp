#include "pycaller.hpp"

#include <pybind11/embed.h>
#include <pybind11/iostream.h>
#include <pybind11/stl.h>

#include <fstream>
#include <mutex>
#include <utility>

#include "pybind11_json/pybind11_json.hpp"

#include "atom/async/pool.hpp"
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/stopwatcher.hpp"

using namespace pybind11::literals;

namespace lithium {
class PythonManager::Impl {
public:
    Impl() {
        LOG_F(INFO, "Initializing Python interpreter.");
        py::print("Initializing Python interpreter.");
    }

    ~Impl() {
        LOG_F(INFO, "Shutting down Python interpreter.");
        py::print("Shutting down Python interpreter.");
    }

    void loadScript(const std::string& script_name, const std::string& alias) {
        std::lock_guard lock(mutex_);
        try {
            LOG_F(INFO, "Loading script: {} with alias: {}", script_name,
                  alias);
            scripts_.emplace(alias, py::module::import(script_name.c_str()));
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Failed to import script '{}': {}", script_name,
                  e.what());
            THROW_RUNTIME_ERROR("Failed to import script '" + script_name +
                                "': " + e.what());
        }
    }

    void unloadScript(const std::string& alias) {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Unloading script with alias: {}", alias);
            scripts_.erase(iter);
        } else {
            LOG_F(ERROR, "Alias '{}' not found.", alias);
            THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
        }
    }

    void reloadScript(const std::string& alias) {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Reloading script with alias: {}", alias);
            py::module importlib = py::module::import("importlib");
            scripts_[alias] = importlib.attr("reload")(iter->second);
        } else {
            LOG_F(ERROR, "Alias '{}' not found.", alias);
            THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
        }
    }

    template <typename ReturnType, typename... Args>
    ReturnType callFunction(const std::string& alias,
                            const std::string& function_name, Args... args) {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Calling function '{}' in script with alias: {}",
                  function_name, alias);
            py::object func = iter->second.attr(function_name.c_str());
            py::object result = func(std::forward<Args>(args)...);
            return result.cast<ReturnType>();
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    template <typename T>
    auto getVariable(const std::string& alias,
                     const std::string& variable_name) -> T {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Getting variable '{}' from script with alias: {}",
                  variable_name, alias);
            py::object value = iter->second.attr(variable_name.c_str());
            return value.cast<T>();
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    void setVariable(const std::string& alias, const std::string& variable_name,
                     const py::object& value) {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Setting variable '{}' in script with alias: {}",
                  variable_name, alias);
            iter->second.attr(variable_name.c_str()) = value;
        } else {
            LOG_F(ERROR, "Alias '{}' not found.", alias);
            THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
        }
    }

    auto getFunctionList(const std::string& alias) -> std::vector<std::string> {
        std::vector<std::string> functions;
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Getting function list from script with alias: {}",
                  alias);
            py::dict dict = iter->second.attr("__dict__");
            for (auto item : dict) {
                if (py::isinstance<py::function>(item.second)) {
                    functions.push_back(py::cast<std::string>(item.first));
                }
            }
            return functions;
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    auto callMethod(const std::string& alias, const std::string& class_name,
                    const std::string& method_name,
                    const py::args& args = py::args()) -> py::object {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO,
                  "Calling method '{}' of class '{}' in script with alias: {}",
                  method_name, class_name, alias);
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            py::object result = instance.attr(method_name.c_str())(*args);
            return result;
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    auto eval(const std::string& script_content) -> py::object {
        std::lock_guard lock(mutex_);
        try {
            LOG_F(INFO, "Evaluating script content: {}", script_content);
            return py::eval<py::eval_statements>(script_content);
        } catch (const py::error_already_set& e) {
            handleException(e);
            return py::none();
        }
    }

    template <typename T>
    auto getObjectAttribute(const std::string& alias,
                            const std::string& class_name,
                            const std::string& attr_name) -> T {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO,
                  "Getting attribute '{}' of class '{}' from script with "
                  "alias: {}",
                  attr_name, class_name, alias);
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            py::object attr = instance.attr(attr_name.c_str());
            return attr.cast<T>();
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    void setObjectAttribute(const std::string& alias,
                            const std::string& class_name,
                            const std::string& attr_name,
                            const py::object& value) {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(
                INFO,
                "Setting attribute '{}' of class '{}' in script with alias: {}",
                attr_name, class_name, alias);
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            instance.attr(attr_name.c_str()) = value;
        } else {
            LOG_F(ERROR, "Alias '{}' not found.", alias);
            THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
        }
    }

    auto evalExpression(const std::string& alias,
                        const std::string& expression) -> py::object {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO, "Evaluating expression '{}' in script with alias: {}",
                  expression, alias);
            py::object result =
                py::eval(expression, iter->second.attr("__dict__"));
            return result;
        }
        LOG_F(ERROR, "Alias '{}' not found.", alias);
        THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
    }

    auto callFunctionWithListReturn(
        const std::string& alias, const std::string& function_name,
        const std::vector<int>& input_list) -> std::vector<int> {
        std::lock_guard lock(mutex_);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            LOG_F(INFO,
                  "Calling function '{}' with list return in script with "
                  "alias: {}",
                  function_name, alias);
            py::object func = iter->second.attr(function_name.c_str());
            py::object result = func(input_list);
            if (py::isinstance<py::list>(result)) {
                return result.cast<std::vector<int>>();
            } else {
                LOG_F(ERROR, "Function did not return a list.");
                THROW_RUNTIME_ERROR("Function did not return a list.");
            }
        } else {
            LOG_F(ERROR, "Alias '{}' not found.", alias);
            THROW_RUNTIME_ERROR("Alias '" + alias + "' not found.");
        }
    }

    auto listScripts() const -> std::vector<std::string> {
        std::lock_guard lock(mutex_);
        std::vector<std::string> aliases;
        for (const auto& pair : scripts_) {
            aliases.push_back(pair.first);
        }
        LOG_F(INFO, "Listing all loaded scripts.");
        return aliases;
    }

    static void addSysPath(const std::string& path) {
        LOG_F(INFO, "Adding '{}' to sys.path", path);
        py::module sys = py::module::import("sys");
        py::list sysPath = sys.attr("path");
        sysPath.append(path);
    }

    static void syncVariableToPython(const std::string& name,
                                     const py::object& value) {
        LOG_F(INFO, "Syncing variable '{}' to Python", name);
        py::globals()[name.c_str()] = value;
    }

    static auto syncVariableFromPython(const std::string& name) -> py::object {
        auto globalVars = py::globals();
        if (globalVars.contains(name)) {
            LOG_F(INFO, "Syncing variable '{}' from Python", name);
            return globalVars[name.c_str()];
        }
        LOG_F(ERROR, "Global variable '{}' not found.", name);
        THROW_RUNTIME_ERROR("Global variable '" + name + "' not found.");
    }

    static void executeScriptMultithreaded(
        const std::vector<std::string>& scripts) {
        atom::async::ThreadPool<> pool;
        std::vector<std::future<void>> futures;
        futures.reserve(scripts.size());
        for (const auto& script : scripts) {
            futures.emplace_back(pool.enqueue([script]() {
                try {
                    LOG_F(INFO, "Executing script in multithreaded mode.");
                    py::exec(script);
                } catch (const py::error_already_set& e) {
                    handleException(e);
                }
            }));
        }
        for (auto& future : futures) {
            future.get();
        }
    }

    static void executeWithProfiling(const std::string& script_content) {
        atom::utils::StopWatcher sw;
        sw.start();
        try {
            LOG_F(INFO, "Executing script with profiling.");
            py::exec(script_content);
        } catch (const py::error_already_set& e) {
            handleException(e);
        }
        sw.stop();
        auto duration = sw.getAverageLapTime();
        LOG_F(INFO, "Execution time: {} seconds.", duration);
    }

    static void injectCode(const std::string& code_snippet) {
        try {
            LOG_F(INFO, "Injecting code snippet.");
            py::exec(code_snippet);
        } catch (const py::error_already_set& e) {
            handleException(e);
        }
    }

    static void registerFunction(const std::string& name,
                                 std::function<void()> func) {
        LOG_F(INFO, "Registering function '{}'.", name);
        py::globals()[name.c_str()] = py::cpp_function(func);
    }

    static auto getMemoryUsage() -> py::object {
        LOG_F(INFO, "Getting memory usage.");
        py::module gc = py::module::import("gc");
        return gc.attr("get_objects")();
    }

    static void handleException(const py::error_already_set& e) {
        LOG_F(ERROR, "Python Exception: {}", e.what());
        std::cerr << "Python Exception:\n" << e.what() << "\n";
        py::module traceback = py::module::import("traceback");
        py::object tb = traceback.attr("format_exc")();
        LOG_F(ERROR, "Traceback:\n{}", py::cast<std::string>(tb));
    }

    static void executeScriptWithLogging(const std::string& script_content,
                                         const std::string& log_file) {
        std::ofstream logStream(log_file, std::ios::app);
        if (!logStream.is_open()) {
            LOG_F(ERROR, "Cannot open log file: {}", log_file);
            THROW_RUNTIME_ERROR("Cannot open log file: " + log_file);
        }
        py::scoped_ostream_redirect redirect(logStream);
        try {
            LOG_F(INFO, "Executing script with logging to file: {}", log_file);
            py::exec(script_content);
        } catch (const py::error_already_set& e) {
            handleException(e);
        }
    }

private:
    py::scoped_interpreter guard_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, py::module> scripts_;
};

PythonManager::PythonManager() : impl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "PythonManager constructor called.");
}

PythonManager::~PythonManager() {
    LOG_F(INFO, "PythonManager destructor called.");
}

PythonManager::PythonManager(PythonManager&&) noexcept = default;

PythonManager& PythonManager::operator=(PythonManager&&) noexcept = default;

void PythonManager::loadScript(const std::string& script_name,
                               const std::string& alias) {
    impl_->loadScript(script_name, alias);
}

void PythonManager::unloadScript(const std::string& alias) {
    impl_->unloadScript(alias);
}

void PythonManager::reloadScript(const std::string& alias) {
    impl_->reloadScript(alias);
}

template <typename ReturnType, typename... Args>
auto PythonManager::callFunction(const std::string& alias,
                                 const std::string& function_name,
                                 Args... args) -> ReturnType {
    return impl_->callFunction<ReturnType>(alias, function_name,
                                           std::forward<Args>(args)...);
}

template <typename ReturnType>
auto PythonManager::eval(const std::string& script_content) -> ReturnType {
    auto res = impl_->eval(script_content);
    if (res.is_none()) {
        return ReturnType();
    }
    return res.cast<ReturnType>();
}

template <typename T>
auto PythonManager::getVariable(const std::string& alias,
                                const std::string& variable_name) -> T {
    return impl_->getVariable<T>(alias, variable_name);
}

void PythonManager::setVariable(const std::string& alias,
                                const std::string& variable_name,
                                const py::object& value) {
    impl_->setVariable(alias, variable_name, value);
}

auto PythonManager::getFunctionList(const std::string& alias)
    -> std::vector<std::string> {
    return impl_->getFunctionList(alias);
}

template <typename ReturnType>
auto PythonManager::callMethod(const std::string& alias,
                               const std::string& class_name,
                               const std::string& method_name,
                               const py::args& args) -> ReturnType {
    auto res = impl_->callMethod(alias, class_name, method_name, args);
    if (res.is_none()) {
        return ReturnType();
    }
    return res.cast<ReturnType>();
}

template <typename T>
auto PythonManager::getObjectAttribute(const std::string& alias,
                                       const std::string& class_name,
                                       const std::string& attr_name) -> T {
    return impl_->getObjectAttribute<T>(alias, class_name, attr_name);
}

void PythonManager::setObjectAttribute(const std::string& alias,
                                       const std::string& class_name,
                                       const std::string& attr_name,
                                       const py::object& value) {
    impl_->setObjectAttribute(alias, class_name, attr_name, value);
}

py::object PythonManager::evalExpression(const std::string& alias,
                                         const std::string& expression) {
    return impl_->evalExpression(alias, expression);
}

auto PythonManager::callFunctionWithListReturn(
    const std::string& alias, const std::string& function_name,
    const std::vector<int>& input_list) -> std::vector<int> {
    return impl_->callFunctionWithListReturn(alias, function_name, input_list);
}

auto PythonManager::listScripts() const -> std::vector<std::string> {
    return impl_->listScripts();
}

void PythonManager::addSysPath(const std::string& path) {
    impl_->addSysPath(path);
}

void PythonManager::syncVariableToPython(const std::string& name,
                                         py::object value) {
    impl_->syncVariableToPython(name, value);
}

auto PythonManager::syncVariableFromPython(const std::string& name)
    -> py::object {
    return impl_->syncVariableFromPython(name);
}

void PythonManager::executeScriptMultithreaded(
    const std::vector<std::string>& scripts) {
    impl_->executeScriptMultithreaded(scripts);
}

void PythonManager::executeWithProfiling(const std::string& script_content) {
    impl_->executeWithProfiling(script_content);
}

void PythonManager::injectCode(const std::string& code_snippet) {
    impl_->injectCode(code_snippet);
}

void PythonManager::registerFunction(const std::string& name,
                                     std::function<void()> func) {
    impl_->registerFunction(name, std::move(func));
}

auto PythonManager::getMemoryUsage() -> py::object {
    return impl_->getMemoryUsage();
}

void PythonManager::handleException(const py::error_already_set& e) {
    Impl::handleException(e);
}

void PythonManager::executeScriptWithLogging(const std::string& script_content,
                                             const std::string& log_file) {
    impl_->executeScriptWithLogging(script_content, log_file);
}

// 显式实例化模板函数
template void PythonManager::callFunction<void>(const std::string&,
                                                const std::string&);
template void PythonManager::callFunction<void, const std::string&>(
    const std::string&, const std::string&, const std::string&);
template int PythonManager::callFunction<int>(const std::string&,
                                              const std::string&);
template double PythonManager::callFunction<double>(const std::string&,
                                                    const std::string&);
template std::string PythonManager::callFunction<std::string>(
    const std::string&, const std::string&);
template int PythonManager::callMethod<int>(const std::string&,
                                            const std::string&,
                                            const std::string&,
                                            const py::args&);
template double PythonManager::callMethod<double>(const std::string&,
                                                  const std::string&,
                                                  const std::string&,
                                                  const py::args&);
template std::string PythonManager::callMethod<std::string>(const std::string&,
                                                            const std::string&,
                                                            const std::string&,
                                                            const py::args&);
template int PythonManager::eval<int>(const std::string&);
template double PythonManager::eval<double>(const std::string&);
template std::string PythonManager::eval<std::string>(const std::string&);
template int PythonManager::getVariable<int>(const std::string&,
                                             const std::string&);
template double PythonManager::getVariable<double>(const std::string&,
                                                   const std::string&);
template std::string PythonManager::getVariable<std::string>(
    const std::string&, const std::string&);
template int PythonManager::getObjectAttribute<int>(const std::string&,
                                                    const std::string&,
                                                    const std::string&);
template double PythonManager::getObjectAttribute<double>(const std::string&,
                                                          const std::string&,
                                                          const std::string&);
template std::string PythonManager::getObjectAttribute<std::string>(
    const std::string&, const std::string&, const std::string&);

}  // namespace lithium