// yaml2json.hpp
#ifndef YAML2JSON_HPP
#define YAML2JSON_HPP

#include "converter.hpp"
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace lithium::cxxtools::detail {

class Yaml2Json : public Converter<Yaml2Json> {
public:
    nlohmann::json convertImpl(std::string_view yamlFilePath);

    bool saveToFileImpl(const nlohmann::json& jsonData, std::string_view jsonFilePath);
};

} // namespace lithium::cxxtools::detail

#endif // YAML2JSON_HPP