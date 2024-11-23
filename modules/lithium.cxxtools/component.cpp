/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-26

Description: Some useful tools written in c++

**************************************************/

#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/log/loguru.hpp"

#include "csv2json.hpp"
#include "ini2json.hpp"
#include "json2csv.hpp"
#include "json2ini.hpp"
#include "json2xml.hpp"
#include "json2yaml.hpp"
#include "pci_generator.hpp"
#include "xml2json.hpp"
#include "yaml2json.hpp"

using namespace lithium::cxxtools::detail;
using namespace lithium::cxxtools::converters;

ATOM_MODULE(lithium_image, [](Component& com) {
    LOG_F(INFO, "Lithium Image Component Constructed");

    com.def("csv2json", &Csv2Json::convert, "utils",
            "Convert a CSV file to JSON format");
    com.def("json2csv", &JsonToCsvConverter::convert, "utils",
            "Convert a JSON file to CSV format");
    com.def("ini2json", &Ini2Json::convert, "utils",
            "Convert an INI file to JSON format");
    com.def("json2ini", &JsonToIniConverter::convert, "utils",
            "Convert a JSON file to INI format");
    com.def("json2xml", &JsonToXmlConverter::convert, "utils",
            "Convert a JSON file to XML format");
    com.def("xml2json", &Xml2Json::convert, "utils",
            "Convert an XML file to JSON format");
    com.def("yaml2json", &Yaml2Json::convert, "utils",
            "Convert a YAML file to JSON format");
    com.def("json2yaml", &JsonToYamlConverter::convert, "utils",

            "Convert a JSON file to YAML format");

    com.def("generate_pci", &parseAndGeneratePCIInfo, "utils",
            "Generate PCI device ID");
});