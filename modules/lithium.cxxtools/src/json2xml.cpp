#include "json2xml.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::cxxtools::converters {

/**
 * @brief Recursively converts JSON data to XML elements.
 *
 * @param jsonData The JSON data.
 * @param xmlElement The current XML element.
 */
void jsonToXml(const nlohmann::json& jsonData,
               tinyxml2::XMLElement* xmlElement) {
    tinyxml2::XMLDocument* xmlDoc = xmlElement->GetDocument();

    for (const auto& item : jsonData.items()) {
        if (item.value().is_object()) {
            tinyxml2::XMLElement* childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            xmlElement->InsertEndChild(childXmlElement);
            jsonToXml(item.value(), childXmlElement);
        } else if (item.value().is_array()) {
            for (const auto& arrayItem : item.value()) {
                tinyxml2::XMLElement* childXmlElement =
                    xmlDoc->NewElement(item.key().c_str());
                xmlElement->InsertEndChild(childXmlElement);
                jsonToXml(arrayItem, childXmlElement);
            }
        } else if (item.value().is_string()) {
            tinyxml2::XMLElement* childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<std::string>().c_str());
            xmlElement->InsertEndChild(childXmlElement);
        } else if (item.value().is_number()) {
            tinyxml2::XMLElement* childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(
                std::to_string(item.value().get<double>()).c_str());
            xmlElement->InsertEndChild(childXmlElement);
        } else if (item.value().is_boolean()) {
            tinyxml2::XMLElement* childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<bool>() ? "true"
                                                              : "false");
            xmlElement->InsertEndChild(childXmlElement);
        } else {
            LOG_F(WARNING, "Unsupported JSON type for key '{}'", item.key());
            tinyxml2::XMLElement* childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText("null");
            xmlElement->InsertEndChild(childXmlElement);
        }
    }
}

bool JsonToXmlConverter::convertImpl(const nlohmann::json& jsonData,
                                     const std::filesystem::path& outputPath) {
    LOG_F(INFO, "Starting JSON to XML conversion.");

    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLElement* rootElement = xmlDoc.NewElement("root");
    xmlDoc.InsertFirstChild(rootElement);

    try {
        jsonToXml(jsonData, rootElement);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during JSON to XML conversion: {}", e.what());
        THROW_RUNTIME_ERROR("Exception during JSON to XML conversion: {}",
                            e.what());
    }

    tinyxml2::XMLError eResult = xmlDoc.SaveFile(outputPath.string().c_str());
    if (eResult != tinyxml2::XML_SUCCESS) {
        LOG_F(ERROR, "Failed to save XML file: {}", outputPath.string());
        THROW_RUNTIME_ERROR("Failed to save XML file: {}", outputPath.string());
    }

    LOG_F(INFO, "Successfully converted JSON to XML: {}", outputPath.string());
    return true;
}

}  // namespace lithium::cxxtools::converters