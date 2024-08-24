#ifndef LITHIUM_SCRIPT_PYCALLER_HPP
#define LITHIUM_SCRIPT_PYCALLER_HPP

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace py = pybind11;

class PythonWrapper {
public:
    PythonWrapper();

    // 新增: 管理多个脚本
    void load_script(const std::string& script_name, const std::string& alias);
    void unload_script(const std::string& alias);
    void reload_script(const std::string& alias);

    template <typename ReturnType, typename... Args>
    ReturnType call_function(const std::string& alias,
                             const std::string& function_name, Args... args);

    template <typename T>
    T get_variable(const std::string& alias, const std::string& variable_name);

    void set_variable(const std::string& alias,
                      const std::string& variable_name,
                      const py::object& value);

    std::vector<std::string> get_function_list(const std::string& alias);

    py::object call_method(const std::string& alias,
                           const std::string& class_name,
                           const std::string& method_name, py::args args);

    template <typename T>
    T get_object_attribute(const std::string& alias,
                           const std::string& class_name,
                           const std::string& attr_name);

    void set_object_attribute(const std::string& alias,
                              const std::string& class_name,
                              const std::string& attr_name,
                              const py::object& value);

    py::object eval_expression(const std::string& alias,
                               const std::string& expression);

    std::vector<int> call_function_with_list_return(
        const std::string& alias, const std::string& function_name,
        const std::vector<int>& input_list);

private:
    py::scoped_interpreter guard{};
    std::map<std::string, py::module> scripts;  // 使用一个map来管理多个脚本
};

template <typename ReturnType, typename... Args>
ReturnType PythonWrapper::call_function(const std::string& alias,
                                        const std::string& function_name,
                                        Args... args) {
    try {
        py::object result =
            scripts.at(alias).attr(function_name.c_str())(args...);
        return result.cast<ReturnType>();
    } catch (const py::error_already_set& e) {
        std::cerr << "Error calling function: " << e.what() << std::endl;
        throw;
    }
}

template <typename T>
T PythonWrapper::get_variable(const std::string& alias,
                              const std::string& variable_name) {
    try {
        py::object var = scripts.at(alias).attr(variable_name.c_str());
        return var.cast<T>();
    } catch (const py::error_already_set& e) {
        std::cerr << "Error getting variable: " << e.what() << std::endl;
        throw;
    }
}

template <typename T>
T PythonWrapper::get_object_attribute(const std::string& alias,
                                      const std::string& class_name,
                                      const std::string& attr_name) {
    try {
        py::object py_class = scripts.at(alias).attr(class_name.c_str());
        py::object instance = py_class();
        py::object attr = instance.attr(attr_name.c_str());
        return attr.cast<T>();
    } catch (const py::error_already_set& e) {
        std::cerr << "Error getting object attribute: " << e.what()
                  << std::endl;
        throw;
    }
}

#endif  // LITHIUM_SCRIPT_PYCALLER_HPP
