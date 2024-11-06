#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "config/configor.hpp"

namespace py = pybind11;
using namespace lithium;

PYBIND11_MODULE(configor, m) {
    py::class_<ConfigManager, std::shared_ptr<ConfigManager>>(m,
                                                              "ConfigManager")
        .def(py::init<>())
        .def_static("create_shared", &ConfigManager::createShared,
                    "Creates a shared pointer instance of ConfigManager.")
        .def_static("create_unique", &ConfigManager::createUnique,
                    "Creates a unique pointer instance of ConfigManager.")
        .def("get_value", &ConfigManager::getValue, py::arg("key_path"),
             "Retrieves the value associated with the given key path.")
        .def("set_value", &ConfigManager::setValue, py::arg("key_path"),
             py::arg("value"), "Sets the value for the specified key path.")
        .def("append_value", &ConfigManager::appendValue, py::arg("key_path"),
             py::arg("value"),
             "Appends a value to an array at the specified key path.")
        .def("delete_value", &ConfigManager::deleteValue, py::arg("key_path"),
             "Deletes the value associated with the given key path.")
        .def("has_value", &ConfigManager::hasValue, py::arg("key_path"),
             "Checks if a value exists for the given key path.")
        .def("get_keys", &ConfigManager::getKeys,
             "Retrieves all keys in the configuration.")
        .def("list_paths", &ConfigManager::listPaths,
             "Lists all configuration files in specified directory.")
        .def("load_from_file", &ConfigManager::loadFromFile, py::arg("path"),
             "Loads configuration data from a file.")
        .def("load_from_dir", &ConfigManager::loadFromDir, py::arg("dir_path"),
             py::arg("recursive") = false,
             "Loads configuration data from a directory.")
        .def("save_to_file", &ConfigManager::saveToFile, py::arg("file_path"),
             "Saves the current configuration to a file.")
        .def("tidy_config", &ConfigManager::tidyConfig,
             "Cleans up the configuration by removing unused entries or "
             "optimizing data.")
        .def("clear_config", &ConfigManager::clearConfig,
             "Clears all configuration data.")
        .def("merge_config",
             py::overload_cast<const json&>(&ConfigManager::mergeConfig),
             py::arg("src"),
             "Merges the current configuration with the provided JSON data.")
        .def("async_load_from_file", &ConfigManager::asyncLoadFromFile,
             py::arg("path"), py::arg("callback"),
             "Asynchronously loads configuration data from a file.")
        .def("async_save_to_file", &ConfigManager::asyncSaveToFile,
             py::arg("file_path"), py::arg("callback"),
             "Asynchronously saves the current configuration to a file.");
}