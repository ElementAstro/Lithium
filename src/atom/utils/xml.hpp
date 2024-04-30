/*
 * xml.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: A XML reader class using tinyxml2.

**************************************************/

#ifndef ATOM_UTILS_XML_HPP
#define ATOM_UTILS_XML_HPP

#include <tinyxml2.h>
#include <string>
#include <vector>

namespace atom::utils {
/**
 * @brief A class for reading and manipulating data from an XML file.
 */
class XMLReader {
public:
    /**
     * @brief Constructs an XMLReader object with the specified file path.
     *
     * @param filePath The path to the XML file to read.
     */
    explicit XMLReader(const std::string &filePath);

    /**
     * @brief Returns the names of all child elements of the specified parent
     * element.
     *
     * @param parentElementName The name of the parent element.
     * @return A vector containing the names of all child elements.
     */
    std::vector<std::string> getChildElementNames(
        const std::string &parentElementName) const;

    /**
     * @brief Returns the text value of the specified element.
     *
     * @param elementName The name of the element.
     * @return The text value of the element.
     */
    std::string getElementText(const std::string &elementName) const;

    /**
     * @brief Returns the value of the specified attribute of the specified
     * element.
     *
     * @param elementName The name of the element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     */
    std::string getAttributeValue(const std::string &elementName,
                                  const std::string &attributeName) const;

    /**
     * @brief Returns the names of all root elements in the XML file.
     *
     * @return A vector containing the names of all root elements.
     */
    std::vector<std::string> getRootElementNames() const;

    /**
     * @brief Checks if the specified parent element has a child element with
     * the specified name.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @return true if the child element exists, false otherwise.
     */
    bool hasChildElement(const std::string &parentElementName,
                         const std::string &childElementName) const;

    /**
     * @brief Returns the text value of the specified child element of the
     * specified parent element.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @return The text value of the child element.
     */
    std::string getChildElementText(const std::string &parentElementName,
                                    const std::string &childElementName) const;

    /**
     * @brief Returns the value of the specified attribute of the specified
     * child element of the specified parent element.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     */
    std::string getChildElementAttributeValue(
        const std::string &parentElementName,
        const std::string &childElementName,
        const std::string &attributeName) const;

    /**
     * @brief Returns the text value of the element specified by a given path.
     *
     * @param path The path to the element.
     * @return The text value of the element.
     * @throw std::runtime_error if the element does not exist.
     */
    std::string getValueByPath(const std::string &path) const;

    /**
     * @brief Returns the value of the specified attribute of the element
     * specified by a given path.
     *
     * @param path The path to the element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     * @throw std::runtime_error if the element does not exist.
     */
    std::string getAttributeValueByPath(const std::string &path,
                                        const std::string &attributeName) const;

    /**
     * @brief Checks if the element specified by a given path has a child
     * element with the specified name.
     *
     * @param path The path to the parent element.
     * @param childElementName The name of the child element.
     * @return true if the child element exists, false otherwise.
     * @throw std::runtime_error if the parent element does not exist.
     */
    bool hasChildElementByPath(const std::string &path,
                               const std::string &childElementName) const;

    /**
     * @brief Returns the text value of the child element with the specified
     * name of the element specified by a given path.
     *
     * @param path The path to the parent element.
     * @param childElementName The name of the child element.
     * @return The text value of the child element.
     * @throw std::runtime_error if the parent or child element does not exist.
     */
    std::string getChildElementTextByPath(
        const std::string &path, const std::string &childElementName) const;

    /**
     * @brief Returns the value of the specified attribute of the child element
     * with the specified name of the element specified by a given path.
     *
     * @param path The path to the parent element.
     * @param childElementName The name of the child element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     * @throw std::runtime_error if the parent or child element does not exist.
     */
    std::string getChildElementAttributeValueByPath(
        const std::string &path, const std::string &childElementName,
        const std::string &attributeName) const;

    /**
     * @brief Saves the XML document to the specified file.
     *
     * @param filePath The path to save the XML document to.
     * @return true if the document was saved successfully, false otherwise.
     */
    bool saveToFile(const std::string &filePath) const;

private:
    mutable tinyxml2::XMLDocument doc_;

    /**
     * @brief Returns a pointer to the element specified by a given path.
     *
     * @param path The path to the element.
     * @return A pointer to the element.
     * @throw std::runtime_error if the element does not exist.
     */
    tinyxml2::XMLElement *getElementByPath(const std::string &path) const;
};
}  // namespace atom::utils

#endif
