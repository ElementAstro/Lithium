// xml2json.cpp
#include "xml2json.hpp"
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

using json = nlohmann::json;

namespace lithium::cxxtools::detail {

void Xml2Json::xmlToJson(const tinyxml2::XMLElement* xmlElement, json& jsonData) {
    for (const tinyxml2::XMLNode* childNode = xmlElement->FirstChild(); childNode != nullptr; childNode = childNode->NextSibling()) {
        if (childNode->ToElement() != nullptr) {
            const std::string childName = childNode->Value();
            json& jsonChild = jsonData[childName];
            if (!jsonChild.is_null()) {
                if (!jsonChild.is_array()) {
                    jsonChild = json::array({jsonChild});
                }
            } else {
                jsonChild = json::array();
            }

            json jsonItem;
            xmlToJson(childNode->ToElement(), jsonItem);
            jsonChild.push_back(jsonItem);
        } else if (childNode->ToText() != nullptr) {
            jsonData = json(childNode->ToText()->Value());
        }
    }
}

bool Xml2Json::convertXmlToJson(const std::string& xmlFilePath, json& jsonData) {
    LOG_F(INFO, "Reading XML file: {}", xmlFilePath);
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.LoadFile(xmlFilePath.c_str()) != tinyxml2::XML_SUCCESS) {
        LOG_F(ERROR, "Failed to load XML file: {}", xmlFilePath);
        return false;
    }

    LOG_F(INFO, "Converting XML to JSON");
    xmlToJson(xmlDoc.RootElement(), jsonData);
    return true;
}

json Xml2Json::convertImpl(std::string_view xmlFilePath) {
    json jsonData;
    if (!convertXmlToJson(std::string(xmlFilePath), jsonData)) {
        THROW_RUNTIME_ERROR("XML to JSON conversion failed for file: ", xmlFilePath);
    }
    return jsonData;
}

bool Xml2Json::saveToFileImpl(const json& jsonData, std::string_view jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }
    jsonFile << std::setw(4) << jsonData << std::endl;
    return true;
}

} // namespace lithium::cxxtools::detail