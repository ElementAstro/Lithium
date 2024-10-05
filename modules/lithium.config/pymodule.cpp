#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "config/configor.hpp"

namespace py = pybind11;

static auto mConfigManager = lithium::ConfigManager::createShared();

PYBIND11_MODULE(lithium_config, m) {
    py::class_<lithium::ConfigManager, std::shared_ptr<lithium::ConfigManager>>(
        m, "ConfigManager")
        .def("getConfig", &lithium::ConfigManager::getValue)
        .def("setConfig", &lithium::ConfigManager::setValue)
        .def("hasConfig", &lithium::ConfigManager::hasValue)
        .def("deleteConfig", &lithium::ConfigManager::deleteValue)
        .def("loadConfig", &lithium::ConfigManager::loadFromFile)
        .def("loadConfigs", &lithium::ConfigManager::loadFromDir)
        .def("saveConfig", &lithium::ConfigManager::saveToFile)
        .def("tidyConfig", &lithium::ConfigManager::tidyConfig)
        .def("clearConfig", &lithium::ConfigManager::clearConfig)
        .def("asyncLoadConfig", &lithium::ConfigManager::asyncLoadFromFile)
        .def("asyncSaveConfig", &lithium::ConfigManager::asyncSaveToFile);

    m.attr("config_instance") = mConfigManager;
}
