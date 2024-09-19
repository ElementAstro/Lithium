/*
 * xml.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: A XML reader class using tinyxml2.

**************************************************/

#include "xml.hpp"

#include "atom/log/loguru.hpp"

namespace atom::utils {
XMLReader::XMLReader(const std::string &filePath) {
    LOG_F(INFO, "Loading XML file: {}", filePath);
    if (doc_.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
        LOG_F(ERROR, "Failed to load XML file: {}", filePath);
        throw std::runtime_error("Failed to load XML file");
    }
    LOG_F(INFO, "Successfully loaded XML file: {}", filePath);
}

auto XMLReader::getChildElementNames(const std::string &parentElementName) const
    -> std::vector<std::string> {
    LOG_F(INFO, "Getting child element names for parent: {}",
          parentElementName);
    std::vector<std::string> childElementNames;
    const tinyxml2::XMLElement *parentElement =
        doc_.FirstChildElement(parentElementName.c_str());
    if (parentElement != nullptr) {
        for (const tinyxml2::XMLElement *element =
                 parentElement->FirstChildElement();
             element != nullptr; element = element->NextSiblingElement()) {
            childElementNames.emplace_back(element->Name());
        }
    }
    LOG_F(INFO, "Found {} child elements for parent: {}",
          childElementNames.size(), parentElementName);
    return childElementNames;
}

auto XMLReader::getElementText(const std::string &elementName) const
    -> std::string {
    LOG_F(INFO, "Getting text for element: {}", elementName);
    const tinyxml2::XMLElement *element =
        doc_.FirstChildElement(elementName.c_str());
    if (element != nullptr) {
        return element->GetText();
    }
    return "";
}

auto XMLReader::getAttributeValue(const std::string &elementName,
                                  const std::string &attributeName) const
    -> std::string {
    LOG_F(INFO, "Getting attribute value for element: {}, attribute: {}",
          elementName, attributeName);
    const tinyxml2::XMLElement *element =
        doc_.FirstChildElement(elementName.c_str());
    if (element != nullptr) {
        return element->Attribute(attributeName.c_str());
    }
    return "";
}

auto XMLReader::getRootElementNames() const -> std::vector<std::string> {
    LOG_F(INFO, "Getting root element names");
    std::vector<std::string> rootElementNames;
    const tinyxml2::XMLElement *rootElement = doc_.RootElement();
    if (rootElement != nullptr) {
        rootElementNames.emplace_back(rootElement->Name());
    }
    LOG_F(INFO, "Found {} root elements", rootElementNames.size());
    return rootElementNames;
}

auto XMLReader::hasChildElement(const std::string &parentElementName,
                                const std::string &childElementName) const
    -> bool {
    LOG_F(INFO, "Checking if parent element: {} has child element: {}",
          parentElementName, childElementName);
    const tinyxml2::XMLElement *parentElement =
        doc_.FirstChildElement(parentElementName.c_str());
    if (parentElement != nullptr) {
        return parentElement->FirstChildElement(childElementName.c_str()) !=
               nullptr;
    }
    return false;
}

auto XMLReader::getChildElementText(const std::string &parentElementName,
                                    const std::string &childElementName) const
    -> std::string {
    LOG_F(INFO, "Getting text for child element: {} of parent element: {}",
          childElementName, parentElementName);
    const tinyxml2::XMLElement *parentElement =
        doc_.FirstChildElement(parentElementName.c_str());
    if (parentElement != nullptr) {
        const tinyxml2::XMLElement *childElement =
            parentElement->FirstChildElement(childElementName.c_str());
        if (childElement != nullptr) {
            return childElement->GetText();
        }
    }
    return "";
}

auto XMLReader::getChildElementAttributeValue(
    const std::string &parentElementName, const std::string &childElementName,
    const std::string &attributeName) const -> std::string {
    LOG_F(INFO,
          "Getting attribute value for child element: {} of parent element: "
          "{}, attribute: {}",
          childElementName, parentElementName, attributeName);
    const tinyxml2::XMLElement *parentElement =
        doc_.FirstChildElement(parentElementName.c_str());
    if (parentElement != nullptr) {
        const tinyxml2::XMLElement *childElement =
            parentElement->FirstChildElement(childElementName.c_str());
        if (childElement != nullptr) {
            return childElement->Attribute(attributeName.c_str());
        }
    }
    return "";
}

auto XMLReader::getValueByPath(const std::string &path) const -> std::string {
    LOG_F(INFO, "Getting value by path: {}", path);
    std::string value;
    tinyxml2::XMLElement *element = getElementByPath(path);
    if (element != nullptr) {
        value = element->GetText();
    }
    return value;
}

// 根据路径获取属性值
auto XMLReader::getAttributeValueByPath(const std::string &path,
                                        const std::string &attributeName) const
    -> std::string {
    LOG_F(INFO, "Getting attribute value by path: {}, attribute: {}", path,
          attributeName);
    std::string value;
    tinyxml2::XMLElement *element = getElementByPath(path);
    if (element != nullptr) {
        value = element->Attribute(attributeName.c_str());
    }
    return value;
}

// 根据路径判断是否存在子元素
bool XMLReader::hasChildElementByPath(
    const std::string &path, const std::string &childElementName) const {
    LOG_F(INFO, "Checking if path: {} has child element: {}", path,
          childElementName);
    tinyxml2::XMLElement *element = getElementByPath(path);
    if (element != nullptr) {
        return element->FirstChildElement(childElementName.c_str()) != nullptr;
    }
    return false;
}

// 根据路径获取子元素的文本
auto XMLReader::getChildElementTextByPath(
    const std::string &path,
    const std::string &childElementName) const -> std::string {
    LOG_F(INFO, "Getting text for child element: {} by path: {}",
          childElementName, path);
    std::string value;
    tinyxml2::XMLElement *element = getElementByPath(path);
    if (element != nullptr) {
        tinyxml2::XMLElement *childElement =
            element->FirstChildElement(childElementName.c_str());
        if (childElement != nullptr) {
            value = childElement->GetText();
        }
    }
    return value;
}

auto XMLReader::getChildElementAttributeValueByPath(
    const std::string &path, const std::string &childElementName,
    const std::string &attributeName) const -> std::string {
    LOG_F(INFO,
          "Getting attribute value for child element: {} by path: {}, "
          "attribute: {}",
          childElementName, path, attributeName);
    std::string value;
    tinyxml2::XMLElement *element = getElementByPath(path);
    if (element != nullptr) {
        tinyxml2::XMLElement *childElement =
            element->FirstChildElement(childElementName.c_str());
        if (childElement != nullptr) {
            value = childElement->Attribute(attributeName.c_str());
        }
    }
    return value;
}

auto XMLReader::saveToFile(const std::string &filePath) const -> bool {
    LOG_F(INFO, "Saving XML to file: {}", filePath);
    return doc_.SaveFile(filePath.c_str()) == tinyxml2::XML_SUCCESS;
}

tinyxml2::XMLElement *XMLReader::getElementByPath(
    const std::string &path) const {
    LOG_F(INFO, "Getting element by path: {}", path);
    tinyxml2::XMLElement *element = doc_.RootElement();
    size_t pos = 0;
    while ((element != nullptr) && pos != std::string::npos) {
        size_t newPos = path.find('.', pos);
        std::string elementName = path.substr(pos, newPos - pos);
        element = element->FirstChildElement(elementName.c_str());
        pos = (newPos == std::string::npos) ? newPos : newPos + 1;
    }
    return element;
}
}  // namespace atom::utils
