#ifndef LITHIUM_SCRIPT_PYCALLER_HPP
#define LITHIUM_SCRIPT_PYCALLER_HPP

#include <pybind11/pybind11.h>
#include <loguru.hpp>
#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;

namespace lithium {

/**
 * @class PythonWrapper
 * @brief A wrapper class to manage and interact with Python scripts.
 */
class PythonWrapper {
public:
    /**
     * @brief Constructs a new PythonWrapper object.
     */
    PythonWrapper();

    /**
     * @brief Destroys the PythonWrapper object.
     */
    ~PythonWrapper();

    // Disable copy
    PythonWrapper(const PythonWrapper&) = delete;
    PythonWrapper& operator=(const PythonWrapper&) = delete;

    // Enable move
    PythonWrapper(PythonWrapper&&) noexcept;
    PythonWrapper& operator=(PythonWrapper&&) noexcept;

    /**
     * @brief Loads a Python script and assigns it an alias.
     * @param script_name The name of the Python script to load.
     * @param alias The alias to assign to the loaded script.
     */
    void load_script(const std::string& script_name, const std::string& alias);

    /**
     * @brief Unloads a Python script by its alias.
     * @param alias The alias of the script to unload.
     */
    void unload_script(const std::string& alias);

    /**
     * @brief Reloads a Python script by its alias.
     * @param alias The alias of the script to reload.
     */
    void reload_script(const std::string& alias);

    /**
     * @brief Calls a function in a loaded Python script.
     * @tparam ReturnType The return type of the function.
     * @tparam Args The types of the arguments to pass to the function.
     * @param alias The alias of the script containing the function.
     * @param function_name The name of the function to call.
     * @param args The arguments to pass to the function.
     * @return The result of the function call.
     */
    template <typename ReturnType, typename... Args>
    ReturnType call_function(const std::string& alias,
                             const std::string& function_name, Args... args);

    /**
     * @brief Gets a variable from a loaded Python script.
     * @tparam T The type of the variable.
     * @param alias The alias of the script containing the variable.
     * @param variable_name The name of the variable to get.
     * @return The value of the variable.
     */
    template <typename T>
    T get_variable(const std::string& alias, const std::string& variable_name);

    /**
     * @brief Sets a variable in a loaded Python script.
     * @param alias The alias of the script containing the variable.
     * @param variable_name The name of the variable to set.
     * @param value The value to set the variable to.
     */
    void set_variable(const std::string& alias,
                      const std::string& variable_name,
                      const py::object& value);

    /**
     * @brief Gets a list of functions in a loaded Python script.
     * @param alias The alias of the script.
     * @return A vector of function names.
     */
    std::vector<std::string> get_function_list(const std::string& alias);

    /**
     * @brief Calls a method of a class in a loaded Python script.
     * @param alias The alias of the script containing the class.
     * @param class_name The name of the class.
     * @param method_name The name of the method to call.
     * @param args The arguments to pass to the method.
     * @return The result of the method call.
     */
    py::object call_method(const std::string& alias,
                           const std::string& class_name,
                           const std::string& method_name,
                           const py::args& args);

    /**
     * @brief Gets an attribute of an object in a loaded Python script.
     * @tparam T The type of the attribute.
     * @param alias The alias of the script containing the object.
     * @param class_name The name of the class of the object.
     * @param attr_name The name of the attribute to get.
     * @return The value of the attribute.
     */
    template <typename T>
    T get_object_attribute(const std::string& alias,
                           const std::string& class_name,
                           const std::string& attr_name);

    /**
     * @brief Sets an attribute of an object in a loaded Python script.
     * @param alias The alias of the script containing the object.
     * @param class_name The name of the class of the object.
     * @param attr_name The name of the attribute to set.
     * @param value The value to set the attribute to.
     */
    void set_object_attribute(const std::string& alias,
                              const std::string& class_name,
                              const std::string& attr_name,
                              const py::object& value);

    /**
     * @brief Evaluates an expression in a loaded Python script.
     * @param alias The alias of the script.
     * @param expression The expression to evaluate.
     * @return The result of the evaluation.
     */
    py::object eval_expression(const std::string& alias,
                               const std::string& expression);

    /**
     * @brief Calls a function in a loaded Python script that returns a list.
     * @param alias The alias of the script containing the function.
     * @param function_name The name of the function to call.
     * @param input_list The list to pass to the function.
     * @return The list returned by the function.
     */
    std::vector<int> call_function_with_list_return(
        const std::string& alias, const std::string& function_name,
        const std::vector<int>& input_list);

    /**
     * @brief Lists all loaded scripts.
     * @return A vector of script aliases.
     */
    std::vector<std::string> list_scripts() const;

    void add_sys_path(const std::string &path) {
        py::module_ sys = py::module_::import("sys");
        py::list sys_path = sys.attr("path");
        sys_path.append(path);
    }

    // 同步变量到 Python 全局变量
    void sync_variable_to_python(const std::string &name, py::object value) {
        py::globals()[name.c_str()] = value;
    }

    // 从 Python 全局变量同步到 C++
    py::object sync_variable_from_python(const std::string &name) {
        return py::globals()[name.c_str()];
    }

    // 多线程运行 Python 脚本
    void execute_script_multithreaded(const std::vector<std::string> &scripts) {
        std::vector<std::thread> threads;
        std::mutex print_mutex;

        for (const auto &script : scripts) {
            threads.emplace_back([&, script]() {
                try {
                    py::exec(script);
                } catch (const py::error_already_set &e) {
                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cerr << "Error in thread: " << e.what() << std::endl;
                }
            });
        }

        for (auto &t : threads) {
            t.join();
        }
    }

    // 执行并统计脚本性能
    void execute_with_profiling(const std::string &script_content) {
        auto start_time = std::chrono::high_resolution_clock::now();

        try {
            py::exec(script_content);
        } catch (const py::error_already_set &e) {
            handle_exception(e);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;
    }

    // 动态代码注入
    void inject_code(const std::string &code_snippet) {
        try {
            py::exec(code_snippet);
        } catch (const py::error_already_set &e) {
            handle_exception(e);
        }
    }

    // Python 调用 C++ 方法支持
    void register_function(const std::string &name, std::function<void()> func) {
        py::globals()[name.c_str()] = py::cpp_function(func);
    }

    // 内存诊断工具
    py::object get_memory_usage() {
        py::module_ gc = py::module_::import("gc");
        return gc.attr("get_objects")();
    }

    // 捕获详细异常信息
    static void handle_exception(const py::error_already_set &e) {
        std::cerr << "Python Exception:\n" << e.what() << "\n";

        py::module_ traceback = py::module_::import("traceback");
        py::object tb = traceback.attr("format_exc")();
        std::cerr << "Traceback:\n" << py::cast<std::string>(tb) << std::endl;
    }

    // 执行带日志支持的脚本
    void execute_script_with_logging(const std::string &script_content, const std::string &log_file) {
        std::ofstream log_stream(log_file, std::ios::app);
        if (!log_stream.is_open()) {
            throw std::runtime_error("Cannot open log file: " + log_file);
        }

        py::scoped_ostream_redirect stream_redirect(log_stream, py::module_::import("sys").attr("stdout"));
        try {
            py::exec(script_content);
        } catch (const py::error_already_set &e) {
            handle_exception(e);
        }
    }

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}  // namespace lithium

#endif  // LITHIUM_SCRIPT_PYCALLER_HPP
