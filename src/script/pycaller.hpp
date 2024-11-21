#ifndef LITHIUM_SCRIPT_PYCALLER_HPP
#define LITHIUM_SCRIPT_PYCALLER_HPP

#include <pybind11/pybind11.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;

namespace lithium {

class PythonManager {
public:
    PythonManager();
    ~PythonManager();

    // 禁用拷贝
    PythonManager(const PythonManager&) = delete;
    auto operator=(const PythonManager&) -> PythonManager& = delete;

    // 允许移动
    PythonManager(PythonManager&&) noexcept;
    auto operator=(PythonManager&&) noexcept -> PythonManager&;

    auto loadScript(const std::string& script_name,
                    const std::string& alias) -> void;
    auto unloadScript(const std::string& alias) -> void;
    auto reloadScript(const std::string& alias) -> void;

    template <typename ReturnType, typename... Args>
    auto callFunction(const std::string& alias,
                      const std::string& function_name,
                      Args... args) -> ReturnType;

    template <typename ReturnType>
    auto eval(const std::string& script_content) -> ReturnType;

    template <typename T>
    auto getVariable(const std::string& alias,
                     const std::string& variable_name) -> T;

    auto setVariable(const std::string& alias, const std::string& variable_name,
                     const py::object& value) -> void;

    auto getFunctionList(const std::string& alias) -> std::vector<std::string>;

    template <typename ReturnType>
    auto callMethod(const std::string& alias, const std::string& class_name,
                    const std::string& method_name,
                    const py::args& args = py::args()) -> ReturnType;

    template <typename T>
    auto getObjectAttribute(const std::string& alias,
                            const std::string& class_name,
                            const std::string& attr_name) -> T;

    auto setObjectAttribute(const std::string& alias,
                            const std::string& class_name,
                            const std::string& attr_name,
                            const py::object& value) -> void;

    auto evalExpression(const std::string& alias,
                        const std::string& expression) -> py::object;

    auto callFunctionWithListReturn(
        const std::string& alias, const std::string& function_name,
        const std::vector<int>& input_list) -> std::vector<int>;

    [[nodiscard]] auto listScripts() const -> std::vector<std::string>;

    auto addSysPath(const std::string& path) -> void;
    auto syncVariableToPython(const std::string& name,
                              py::object value) -> void;
    auto syncVariableFromPython(const std::string& name) -> py::object;
    auto executeScriptMultithreaded(const std::vector<std::string>& scripts)
        -> void;
    auto executeWithProfiling(const std::string& script_content) -> void;
    auto injectCode(const std::string& code_snippet) -> void;
    auto registerFunction(const std::string& name,
                          std::function<void()> func) -> void;
    auto getMemoryUsage() -> py::object;
    static auto handleException(const py::error_already_set& err) -> void;
    auto executeScriptWithLogging(const std::string& script_content,
                                  const std::string& log_file) -> void;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif  // LITHIUM_SCRIPT_PYCALLER_HPP