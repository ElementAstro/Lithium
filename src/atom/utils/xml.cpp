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

#include <stdexcept>

namespace Atom::Utils
{
    XMLReader::XMLReader(const std::string &filePath)
    {
        if (doc_.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS)
        {
            throw std::runtime_error("Failed to load XML file");
        }
    }

    std::vector<std::string> XMLReader::getChildElementNames(const std::string &parentElementName) const
    {
        std::vector<std::string> childElementNames;
        const tinyxml2::XMLElement *parentElement = doc_.FirstChildElement(parentElementName.c_str());
        if (parentElement)
        {
            for (const tinyxml2::XMLElement *element = parentElement->FirstChildElement(); element != nullptr;
                 element = element->NextSiblingElement())
            {
                childElementNames.push_back(element->Name());
            }
        }
        return childElementNames;
    }

    std::string XMLReader::getElementText(const std::string &elementName) const
    {
        const tinyxml2::XMLElement *element = doc_.FirstChildElement(elementName.c_str());
        if (element)
        {
            return element->GetText();
        }
        return "";
    }

    std::string XMLReader::getAttributeValue(const std::string &elementName, const std::string &attributeName) const
    {
        const tinyxml2::XMLElement *element = doc_.FirstChildElement(elementName.c_str());
        if (element)
        {
            return element->Attribute(attributeName.c_str());
        }
        return "";
    }

    std::vector<std::string> XMLReader::getRootElementNames() const
    {
        std::vector<std::string> rootElementNames;
        const tinyxml2::XMLElement *rootElement = doc_.RootElement();
        if (rootElement)
        {
            rootElementNames.push_back(rootElement->Name());
        }
        return rootElementNames;
    }

    bool XMLReader::hasChildElement(const std::string &parentElementName, const std::string &childElementName) const
    {
        const tinyxml2::XMLElement *parentElement = doc_.FirstChildElement(parentElementName.c_str());
        if (parentElement)
        {
            return parentElement->FirstChildElement(childElementName.c_str()) != nullptr;
        }
        return false;
    }

    std::string XMLReader::getChildElementText(const std::string &parentElementName, const std::string &childElementName) const
    {
        const tinyxml2::XMLElement *parentElement = doc_.FirstChildElement(parentElementName.c_str());
        if (parentElement)
        {
            const tinyxml2::XMLElement *childElement = parentElement->FirstChildElement(childElementName.c_str());
            if (childElement)
            {
                return childElement->GetText();
            }
        }
        return "";
    }

    std::string XMLReader::getChildElementAttributeValue(const std::string &parentElementName, const std::string &childElementName,
                                                         const std::string &attributeName) const
    {
        const tinyxml2::XMLElement *parentElement = doc_.FirstChildElement(parentElementName.c_str());
        if (parentElement)
        {
            const tinyxml2::XMLElement *childElement = parentElement->FirstChildElement(childElementName.c_str());
            if (childElement)
            {
                return childElement->Attribute(attributeName.c_str());
            }
        }
        return "";
    }

    std::string XMLReader::getValueByPath(const std::string &path) const
    {
        std::string value;
        tinyxml2::XMLElement *element = getElementByPath(path);
        if (element)
        {
            value = element->GetText();
        }
        return value;
    }

    // 根据路径获取属性值
    std::string XMLReader::getAttributeValueByPath(const std::string &path, const std::string &attributeName) const
    {
        std::string value;
        tinyxml2::XMLElement *element = getElementByPath(path);
        if (element)
        {
            value = element->Attribute(attributeName.c_str());
        }
        return value;
    }

    // 根据路径判断是否存在子元素
    bool XMLReader::hasChildElementByPath(const std::string &path, const std::string &childElementName) const
    {
        tinyxml2::XMLElement *element = getElementByPath(path);
        if (element)
        {
            return element->FirstChildElement(childElementName.c_str()) != nullptr;
        }
        return false;
    }

    // 根据路径获取子元素的文本
    std::string XMLReader::getChildElementTextByPath(const std::string &path, const std::string &childElementName) const
    {
        std::string value;
        tinyxml2::XMLElement *element = getElementByPath(path);
        if (element)
        {
            tinyxml2::XMLElement *childElement = element->FirstChildElement(childElementName.c_str());
            if (childElement)
            {
                value = childElement->GetText();
            }
        }
        return value;
    }

    std::string XMLReader::getChildElementAttributeValueByPath(const std::string &path, const std::string &childElementName,
                                                               const std::string &attributeName) const
    {
        std::string value;
        tinyxml2::XMLElement *element = getElementByPath(path);
        if (element)
        {
            tinyxml2::XMLElement *childElement = element->FirstChildElement(childElementName.c_str());
            if (childElement)
            {
                value = childElement->Attribute(attributeName.c_str());
            }
        }
        return value;
    }

    bool XMLReader::saveToFile(const std::string &filePath) const
    {
        return doc_.SaveFile(filePath.c_str()) == tinyxml2::XML_SUCCESS;
    }

    tinyxml2::XMLElement *XMLReader::getElementByPath(const std::string &path) const
    {
        tinyxml2::XMLElement *element = doc_.RootElement();
        size_t pos = 0;
        while (element && pos != std::string::npos)
        {
            size_t newPos = path.find('.', pos);
            std::string elementName = path.substr(pos, newPos - pos);
            element = element->FirstChildElement(elementName.c_str());
            pos = (newPos == std::string::npos) ? newPos : newPos + 1;
        }
        return element;
    }
}
