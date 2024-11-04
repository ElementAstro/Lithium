/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-26

Description: Some useful tools written in c++

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

#include "csv2json.hpp"
#include "ini2json.hpp"
#include "json2ini.hpp"
#include "json2xml.hpp"
#include "pci_generator.hpp"
#include "xml2json.hpp"
#include "yaml2json.hpp"

using namespace lithium::cxxtools;

ToolsComponent::ToolsComponent(const std::string& name) : Component(name) {
    LOG_F(INFO, "ToolsComponent Constructed");

    def("csv_to_json", &csvToJson, "lithium.cxxtools", "Convert csv to json");
    def("ini_to_json", &iniToJson, "lithium.cxxtools", "Convert ini to json");
    def("json_to_ini", &jsonToIni, "lithium.cxxtools", "Convert json to ini");
    def("json_to_xml", &jsonToXml, "lithium.cxxtools", "Convert json to xml");
    def("xml_to_json", &xmlToJson, "lithium.cxxtools", "Convert xml to json");
    def("yaml_to_json", &yamlToJson, "lithium.cxxtools",
        "Convert yaml to json");
    def("pci_generator", &parseAndGeneratePCIInfo, "lithium.cxxtools",
        "Generate pci id");
}

ToolsComponent::~ToolsComponent() { LOG_F(INFO, "ToolsComponent Destructed"); }

auto ToolsComponent::initialize() -> bool {
    LOG_F(INFO, "ToolsComponent Initialized");
    return true;
}

auto ToolsComponent::destroy() -> bool {
    LOG_F(INFO, "ToolsComponent Destroyed");
    return true;
}
