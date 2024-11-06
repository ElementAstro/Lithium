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

#if __has_include(<tinyxml2.h>)
#include <tinyxml2.h>
#elif __has_include(<tinyxml2/tinyxml2.h>)
#include <tinyxml2/tinyxml2.h>
#endif

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
    auto getChildElementNames(const std::string &parentElementName) const
        -> std::vector<std::string>;

    /**
     * @brief Returns the text value of the specified element.
     *
     * @param elementName The name of the element.
     * @return The text value of the element.
     */
    auto getElementText(const std::string &elementName) const -> std::string;

    /**
     * @brief Returns the value of the specified attribute of the specified
     * element.
     *
     * @param elementName The name of the element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     */
    auto getAttributeValue(const std::string &elementName,
                           const std::string &attributeName) const
        -> std::string;

    /**
     * @brief Returns the names of all root elements in the XML file.
     *
     * @return A vector containing the names of all root elements.
     */
    auto getRootElementNames() const -> std::vector<std::string>;

    /**
     * @brief Checks if the specified parent element has a child element with
     * the specified name.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @return true if the child element exists, false otherwise.
     */
    auto hasChildElement(const std::string &parentElementName,
                         const std::string &childElementName) const -> bool;

    /**
     * @brief Returns the text value of the specified child element of the
     * specified parent element.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @return The text value of the child element.
     */
    auto getChildElementText(const std::string &parentElementName,
                             const std::string &childElementName) const
        -> std::string;

    /**
     * @brief Returns the value of the specified attribute of the specified
     * child element of the specified parent element.
     *
     * @param parentElementName The name of the parent element.
     * @param childElementName The name of the child element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     */
    auto getChildElementAttributeValue(const std::string &parentElementName,
                                       const std::string &childElementName,
                                       const std::string &attributeName) const
        -> std::string;

    /**
     * @brief Returns the text value of the element specified by a given path.
     *
     * @param path The path to the element.
     * @return The text value of the element.
     * @throw std::runtime_error if the element does not exist.
     */
    auto getValueByPath(const std::string &path) const -> std::string;

    /**
     * @brief Returns the value of the specified attribute of the element
     * specified by a given path.
     *
     * @param path The path to the element.
     * @param attributeName The name of the attribute.
     * @return The value of the attribute.
     * @throw std::runtime_error if the element does not exist.
     */
    auto getAttributeValueByPath(const std::string &path,
                                 const std::string &attributeName) const
        -> std::string;

    /**
     * @brief Checks if the element specified by a given path has a child
     * element with the specified name.
     *
     * @param path The path to the parent element.
     * @param childElementName The name of the child element.
     * @return true if the child element exists, false otherwise.
     * @throw std::runtime_error if the parent element does not exist.
     */
    auto hasChildElementByPath(const std::string &path,
                               const std::string &childElementName) const
        -> bool;

    /**
     * @brief Returns the text value of the child element with the specified
     * name of the element specified by a given path.
     *
     * @param path The path to the parent element.
     * @param childElementName The name of the child element.
     * @return The text value of the child element.
     * @throw std::runtime_error if the parent or child element does not exist.
     */
    auto getChildElementTextByPath(const std::string &path,
                                   const std::string &childElementName) const
        -> std::string;

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
    auto getChildElementAttributeValueByPath(
        const std::string &path, const std::string &childElementName,
        const std::string &attributeName) const -> std::string;

    /**
     * @brief Saves the XML document to the specified file.
     *
     * @param filePath The path to save the XML document to.
     * @return true if the document was saved successfully, false otherwise.
     */
    auto saveToFile(const std::string &filePath) const -> bool;

private:
    mutable tinyxml2::XMLDocument doc_;

    /**
     * @brief Returns a pointer to the element specified by a given path.
     *
     * @param path The path to the element.
     * @return A pointer to the element.
     * @throw std::runtime_error if the element does not exist.
     */
    auto getElementByPath(const std::string &path) const
        -> tinyxml2::XMLElement *;
};
}  // namespace atom::utils

#endif
