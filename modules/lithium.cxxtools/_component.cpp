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
#include "xml2json.hpp"

#if ENABLE_TOML
#include "json2toml.hpp"
#include "toml2json.hpp"
#endif

ToolsComponent::ToolsComponent(const std::string& name) : Component(name) {
    LOG_F(INFO, "ToolsComponent Constructed");

    def("csv_to_json", &csv_to_json, "utils", "Convert csv to json");
    def("ini_to_json", &ini_to_json, "utils", "Convert ini to json");
    def("json_to_ini", &json_to_ini, "utils", "Convert json to ini");
    def("json_to_xml", &json_to_xml, "utils", "Convert json to xml");
    def("xml_to_json", &xml_to_json, "utils", "Convert xml to json");
#if ENABLE_TOML
    def("json_to_toml", &json_to_toml, "utils", "Convert json to toml");
    def("toml_to_json", &toml_to_json, "utils", "Convert toml to json");
#endif
}

ToolsComponent::~ToolsComponent() { LOG_F(INFO, "ToolsComponent Destructed"); }

bool ToolsComponent::initialize() {
    LOG_F(INFO, "ToolsComponent Initialized");
    return true;
}

bool ToolsComponent::destroy() {
    LOG_F(INFO, "ToolsComponent Destroyed");
    return true;
}
