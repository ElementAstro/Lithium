#include "pycaller.hpp"

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace py = pybind11;

namespace lithium {
// Implementation class
class PythonWrapper::Impl {
public:
    Impl() { LOG_F(INFO, "Initializing Python interpreter."); }

    ~Impl() { LOG_F(INFO, "Shutting down Python interpreter."); }

    void loadScript(const std::string& script_name, const std::string& alias) {
        LOG_F(INFO, "Loading script '{}' with alias '{}'.", script_name, alias);
        try {
            scripts_.emplace(alias, py::module::import(script_name.c_str()));
            LOG_F(INFO, "Script '{}' loaded successfully.", script_name);
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error loading script '{}': {}", script_name,
                  e.what());
            throw std::runtime_error("Failed to import script '" + script_name +
                                     "': " + e.what());
        }
    }

    void unloadScript(const std::string& alias) {
        LOG_F(INFO, "Unloading script with alias '{}'.", alias);
        auto iter = scripts_.find(alias);
        if (iter != scripts_.end()) {
            scripts_.erase(iter);
            LOG_F(INFO, "Script with alias '{}' unloaded successfully.", alias);
        } else {
            LOG_F(WARNING, "Alias '{}' not found.", alias);
            throw std::runtime_error("Alias '" + alias + "' not found.");
        }
    }

    void reloadScript(const std::string& alias) {
        LOG_F(INFO, "Reloading script with alias '{}'.", alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found for reloading.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::module script = iter->second;
            py::module::import("importlib").attr("reload")(script);
            LOG_F(INFO, "Script with alias '{}' reloaded successfully.", alias);
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error reloading script '{}': {}", alias, e.what());
            throw std::runtime_error("Failed to reload script '" + alias +
                                     "': " + e.what());
        }
    }

    template <typename ReturnType, typename... Args>
    ReturnType callFunction(const std::string& alias,
                            const std::string& function_name, Args... args) {
        LOG_F(INFO, "Calling function '{}' from alias '{}'.", function_name,
              alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object result =
                iter->second.attr(function_name.c_str())(args...);
            LOG_F(INFO, "Function '{}' called successfully.", function_name);
            return result.cast<ReturnType>();
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error calling function '{}': {}", function_name,
                  e.what());
            throw std::runtime_error("Error calling function '" +
                                     function_name + "': " + e.what());
        }
    }

    template <typename T>
    T getVariable(const std::string& alias, const std::string& variable_name) {
        LOG_F(INFO, "Getting variable '{}' from alias '{}'.", variable_name,
              alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object var = iter->second.attr(variable_name.c_str());
            LOG_F(INFO, "Variable '{}' retrieved successfully.", variable_name);
            return var.cast<T>();
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error getting variable '{}': {}", variable_name,
                  e.what());
            throw std::runtime_error("Error getting variable '" +
                                     variable_name + "': " + e.what());
        }
    }

    void setVariable(const std::string& alias, const std::string& variable_name,
                     const py::object& value) {
        LOG_F(INFO, "Setting variable '{}' in alias '{}'.", variable_name,
              alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            iter->second.attr(variable_name.c_str()) = value;
            LOG_F(INFO, "Variable '{}' set successfully.", variable_name);
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error setting variable '{}': {}", variable_name,
                  e.what());
            throw std::runtime_error("Error setting variable '" +
                                     variable_name + "': " + e.what());
        }
    }

    auto getFunctionList(const std::string& alias) -> std::vector<std::string> {
        LOG_F(INFO, "Getting function list from alias '{}'.", alias);
        std::vector<std::string> functions;
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::dict dict = iter->second.attr("__dict__");
            for (auto item : dict) {
                if (py::isinstance<py::function>(item.second)) {
                    functions.emplace_back(py::str(item.first));
                }
            }
            LOG_F(INFO, "Function list retrieved successfully from alias '{}'.",
                  alias);
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error getting function list: {}", e.what());
            throw std::runtime_error("Error getting function list: " +
                                     std::string(e.what()));
        }
        return functions;
    }

    auto callMethod(const std::string& alias, const std::string& class_name,
                    const std::string& method_name,
                    const py::args& args) -> py::object {
        LOG_F(INFO, "Calling method '{}' of class '{}' from alias '{}'.",
              method_name, class_name, alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            py::object result = instance.attr(method_name.c_str())(*args);
            LOG_F(INFO, "Method '{}' called successfully.", method_name);
            return result;
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error calling method '{}': {}", method_name,
                  e.what());
            throw std::runtime_error("Error calling method '" + method_name +
                                     "': " + e.what());
        }
    }

    template <typename T>
    T getObjectAttribute(const std::string& alias,
                         const std::string& class_name,
                         const std::string& attr_name) {
        LOG_F(INFO, "Getting attribute '{}' from class '{}' in alias '{}'.",
              attr_name, class_name, alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            py::object attr = instance.attr(attr_name.c_str());
            LOG_F(INFO, "Attribute '{}' retrieved successfully.", attr_name);
            return attr.cast<T>();
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error getting attribute '{}': {}", attr_name,
                  e.what());
            throw std::runtime_error("Error getting attribute '" + attr_name +
                                     "': " + e.what());
        }
    }

    void setObjectAttribute(const std::string& alias,
                            const std::string& class_name,
                            const std::string& attr_name,
                            const py::object& value) {
        LOG_F(INFO, "Setting attribute '{}' of class '{}' in alias '{}'.",
              attr_name, class_name, alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object pyClass = iter->second.attr(class_name.c_str());
            py::object instance = pyClass();
            instance.attr(attr_name.c_str()) = value;
            LOG_F(INFO, "Attribute '{}' set successfully.", attr_name);
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error setting attribute '{}': {}", attr_name,
                  e.what());
            throw std::runtime_error("Error setting attribute '" + attr_name +
                                     "': " + e.what());
        }
    }

    auto evalExpression(const std::string& alias,
                        const std::string& expression) -> py::object {
        LOG_F(INFO, "Evaluating expression '{}' in alias '{}'.", expression,
              alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::object result =
                py::eval(expression, iter->second.attr("__dict__"));
            LOG_F(INFO, "Expression '{}' evaluated successfully.", expression);
            return result;
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error evaluating expression '{}': {}", expression,
                  e.what());
            throw std::runtime_error("Error evaluating expression '" +
                                     expression + "': " + e.what());
        }
    }

    auto callFunctionWithListReturn(
        const std::string& alias, const std::string& function_name,
        const std::vector<int>& input_list) -> std::vector<int> {
        LOG_F(INFO, "Calling function '{}' with list return from alias '{}'.",
              function_name, alias);
        try {
            auto iter = scripts_.find(alias);
            if (iter == scripts_.end()) {
                LOG_F(WARNING, "Alias '{}' not found.", alias);
                throw std::runtime_error("Alias '" + alias + "' not found.");
            }
            py::list pyList = py::cast(input_list);
            py::object result =
                iter->second.attr(function_name.c_str())(pyList);
            if (!py::isinstance<py::list>(result)) {
                LOG_F(ERROR, "Function '{}' did not return a list.",
                      function_name);
                throw std::runtime_error("Function '" + function_name +
                                         "' did not return a list.");
            }
            auto output = result.cast<std::vector<int>>();
            LOG_F(INFO, "Function '{}' called successfully with list return.",
                  function_name);
            return output;
        } catch (const py::error_already_set& e) {
            LOG_F(ERROR, "Error calling function '{}': {}", function_name,
                  e.what());
            throw std::runtime_error("Error calling function '" +
                                     function_name + "': " + e.what());
        }
    }

    auto listScripts() const -> std::vector<std::string> {
        LOG_F(INFO, "Listing all loaded scripts.");
        std::vector<std::string> aliases;
        aliases.reserve(scripts_.size());
        for (const auto& pair : scripts_) {
            aliases.emplace_back(pair.first);
        }
        LOG_F(INFO, "Total scripts loaded: %zu", aliases.size());
        return aliases;
    }

private:
    py::scoped_interpreter guard_;
    std::unordered_map<std::string, py::module> scripts_;
};

// PythonWrapper Implementation

PythonWrapper::PythonWrapper() : pImpl(std::make_unique<Impl>()) {}

PythonWrapper::~PythonWrapper() = default;

PythonWrapper::PythonWrapper(PythonWrapper&&) noexcept = default;

auto PythonWrapper::operator=(PythonWrapper&&) noexcept -> PythonWrapper& =
                                                               default;

void PythonWrapper::load_script(const std::string& script_name,
                                const std::string& alias) {
    pImpl->loadScript(script_name, alias);
}

void PythonWrapper::unload_script(const std::string& alias) {
    pImpl->unloadScript(alias);
}

void PythonWrapper::reload_script(const std::string& alias) {
    pImpl->reloadScript(alias);
}

template <typename ReturnType, typename... Args>
auto PythonWrapper::call_function(const std::string& alias,
                                  const std::string& function_name,
                                  Args... args) -> ReturnType {
    return pImpl->callFunction<ReturnType>(alias, function_name, args...);
}

template <typename T>
auto PythonWrapper::get_variable(const std::string& alias,
                                 const std::string& variable_name) -> T {
    return pImpl->getVariable<T>(alias, variable_name);
}

void PythonWrapper::set_variable(const std::string& alias,
                                 const std::string& variable_name,
                                 const py::object& value) {
    pImpl->setVariable(alias, variable_name, value);
}

auto PythonWrapper::get_function_list(const std::string& alias)
    -> std::vector<std::string> {
    return pImpl->getFunctionList(alias);
}

auto PythonWrapper::call_method(const std::string& alias,
                                const std::string& class_name,
                                const std::string& method_name,
                                const py::args& args) -> py::object {
    return pImpl->callMethod(alias, class_name, method_name, args);
}

template <typename T>
auto PythonWrapper::get_object_attribute(const std::string& alias,
                                         const std::string& class_name,
                                         const std::string& attr_name) -> T {
    return pImpl->getObjectAttribute<T>(alias, class_name, attr_name);
}

void PythonWrapper::set_object_attribute(const std::string& alias,
                                         const std::string& class_name,
                                         const std::string& attr_name,
                                         const py::object& value) {
    pImpl->setObjectAttribute(alias, class_name, attr_name, value);
}

auto PythonWrapper::eval_expression(
    const std::string& alias, const std::string& expression) -> py::object {
    return pImpl->evalExpression(alias, expression);
}

auto PythonWrapper::call_function_with_list_return(
    const std::string& alias, const std::string& function_name,
    const std::vector<int>& input_list) -> std::vector<int> {
    return pImpl->callFunctionWithListReturn(alias, function_name, input_list);
}

auto PythonWrapper::list_scripts() const -> std::vector<std::string> {
    return pImpl->listScripts();
}

// Explicit template instantiation
template int PythonWrapper::call_function<int>(const std::string&,
                                               const std::string&);
template std::string PythonWrapper::get_variable<std::string>(
    const std::string&, const std::string&);
template int PythonWrapper::get_object_attribute<int>(const std::string&,
                                                      const std::string&,
                                                      const std::string&);
}  // namespace lithium
