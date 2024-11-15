// xml2json.hpp
#ifndef XML2JSON_HPP
#define XML2JSON_HPP

#include "converter.hpp"
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "tinyxml2.h"

namespace lithium::cxxtools::detail {
using json = nlohmann::json;
class Xml2Json : public Converter<Xml2Json> {
public:
    nlohmann::json convertImpl(std::string_view xmlFilePath);

    bool saveToFileImpl(const nlohmann::json& jsonData, std::string_view jsonFilePath);

private:
    void xmlToJson(const tinyxml2::XMLElement* xmlElement, json& jsonData);
    bool convertXmlToJson(const std::string& xmlFilePath, json& jsonData);
};

} // namespace lithium::cxxtools::detail

#endif // XML2JSON_HPP