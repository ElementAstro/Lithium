#include "pycaller.hpp"

#include <iostream>

PythonWrapper::PythonWrapper() {
    // 初始化解释器
}

void PythonWrapper::load_script(const std::string& script_name,
                                const std::string& alias) {
    try {
        scripts[alias] = py::module::import(script_name.c_str());
    } catch (const py::error_already_set& e) {
        std::cerr << "Error importing script: " << e.what() << std::endl;
    }
}

void PythonWrapper::unload_script(const std::string& alias) {
    try {
        scripts.erase(alias);
    } catch (const std::exception& e) {
        std::cerr << "Error unloading script: " << e.what() << std::endl;
    }
}

void PythonWrapper::reload_script(const std::string& alias) {
    try {
        py::module script = scripts.at(alias);
        py::module::import("importlib").attr("reload")(script);
    } catch (const py::error_already_set& e) {
        std::cerr << "Error reloading script: " << e.what() << std::endl;
    }
}

void PythonWrapper::set_variable(const std::string& alias,
                                 const std::string& variable_name,
                                 const py::object& value) {
    try {
        scripts.at(alias).attr(variable_name.c_str()) = value;
    } catch (const py::error_already_set& e) {
        std::cerr << "Error setting variable: " << e.what() << std::endl;
        throw;
    }
}

std::vector<std::string> PythonWrapper::get_function_list(
    const std::string& alias) {
    std::vector<std::string> functions;
    try {
        py::dict dict = scripts.at(alias).attr("__dict__");
        for (auto item : dict) {
            if (py::isinstance<py::function>(item.second)) {
                functions.push_back(py::str(item.first));
            }
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Error getting function list: " << e.what() << std::endl;
    }
    return functions;
}

py::object PythonWrapper::call_method(const std::string& alias,
                                      const std::string& class_name,
                                      const std::string& method_name,
                                      py::args args) {
    try {
        py::object py_class = scripts.at(alias).attr(class_name.c_str());
        py::object instance = py_class();
        return instance.attr(method_name.c_str())(*args);
    } catch (const py::error_already_set& e) {
        std::cerr << "Error calling method: " << e.what() << std::endl;
        throw;
    }
}

void PythonWrapper::set_object_attribute(const std::string& alias,
                                         const std::string& class_name,
                                         const std::string& attr_name,
                                         const py::object& value) {
    try {
        py::object py_class = scripts.at(alias).attr(class_name.c_str());
        py::object instance = py_class();
        instance.attr(attr_name.c_str()) = value;
    } catch (const py::error_already_set& e) {
        std::cerr << "Error setting object attribute: " << e.what()
                  << std::endl;
        throw;
    }
}

py::object PythonWrapper::eval_expression(const std::string& alias,
                                          const std::string& expression) {
    try {
        return py::eval(expression, scripts.at(alias).attr("__dict__"));
    } catch (const py::error_already_set& e) {
        std::cerr << "Error evaluating expression: " << e.what() << std::endl;
        throw;
    }
}

std::vector<int> PythonWrapper::call_function_with_list_return(
    const std::string& alias, const std::string& function_name,
    const std::vector<int>& input_list) {
    try {
        py::list py_list = py::cast(input_list);
        py::object result =
            scripts.at(alias).attr(function_name.c_str())(py_list);
        if (!py::isinstance<py::list>(result)) {
            throw std::runtime_error("Function did not return a list.");
        }
        return result.cast<std::vector<int>>();
    } catch (const py::error_already_set& e) {
        std::cerr << "Error calling function with list return: " << e.what()
                  << std::endl;
        throw;
    }
}
