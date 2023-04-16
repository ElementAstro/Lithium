/*
 * xmlreader.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-4

Description: XML Reader

**************************************************/

#ifndef XML_READER_HPP
#define XML_READER_HPP

#include <string>
#include "pugixml.hpp"
#include "nlohmann/json.hpp"

namespace OpenAPT
{
    namespace XML
    {

        /**
         * @brief Reads an XML file and returns its root node.
         *
         * @param filename The path of the XML file to read.
         * @return The root node of the XML file.
         * @note This function reads an XML file from disk and returns its root node.
         * @note 该函数从磁盘读取XML文件并返回其根节点。
         */
        pugi::xml_node read_xml(const std::string &filename);

        /**
         * @brief Modifies the value of a node based on its path.
         *
         * @param root The root node of the XML document.
         * @param path The path of the node to modify.
         * @param value The new value for the node.
         * @return true if the modification was successful, false otherwise.
         * @note This function modifies the value of an XML node based on its path.
         * @note 该函数基于路径修改XML节点的值。
         */
        bool modify_node(pugi::xml_node &root, const std::string &path, const std::string &value);

        /**
         * @brief Saves an XML file.
         *
         * @param filename The path of the XML file to write.
         * @param root The root node of the XML document.
         * @return true if the write was successful, false otherwise.
         * @note This function saves an XML file to disk.
         * @note 该函数将XML文件保存到磁盘上。
         */
        bool write_xml(const std::string &filename, const pugi::xml_node &root);

        /**
         * @brief Validates an XML file.
         *
         * @param filename The path of the XML file to validate.
         * @return true if the XML file is valid, false otherwise.
         * @note This function validates an XML file against its DTD or schema.
         * @note 该函数根据DTD或模式验证XML文件。
         */
        bool validate_xml(const std::string &filename);

        /**
         * @brief Converts an XML node to a JSON string.
         *
         * @param root The root node of the XML document.
         * @return The JSON string representing the XML node.
         * @note This function converts an XML node to a JSON string.
         * @note 该函数将XML节点转换为JSON字符串。
         */
        std::string xml_to_json(const pugi::xml_node &root);

    } // namespace XML
} // namespace OpenAPT

#endif // XML_READER_HPP
