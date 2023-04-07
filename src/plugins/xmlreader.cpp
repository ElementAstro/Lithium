/*
 * xmlreader.cpp
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

#include "xmlreader.hpp"

#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace OpenAPT
{
    namespace XML
    {

        pugi::xml_node read_xml(const std::string &filename)
        {
            spdlog::info("Reading XML file {}", filename);
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(filename.c_str());
            if (!result)
            {
                spdlog::error("Failed to read XML file {}: {}", filename, result.description());
                return pugi::xml_node();
            }
            return doc.child("root");
        }

        bool modify_node(pugi::xml_node &root, const std::string &path, const std::string &value)
        {
            spdlog::info("Modifying XML node with path {} to value {}", path, value);
            pugi::xpath_node_set nodes = root.select_nodes(path.c_str());
            if (nodes.empty())
            {
                spdlog::error("Failed to find node with path {}", path);
                return false;
            }

            for (auto &node : nodes)
            {
                node.node().text().set(value.c_str());
            }
            return true;
        }

        bool write_xml(const std::string &filename, const pugi::xml_node &root)
        {
            spdlog::info("Writing XML file {}", filename);
            pugi::xml_document doc;
            doc.append_child("root").append_copy(root);

            if (!doc.save_file(filename.c_str()))
            {
                spdlog::error("Failed to write XML file {}", filename);
                return false;
            }
            return true;
        }

        bool validate_xml(const std::string &filename)
        {
            spdlog::info("Validating XML file {}", filename);
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(filename.c_str());
            if (!result)
            {
                spdlog::error("Failed to validate XML file {}: {}", filename, result.description());
                return false;
            }
            return true;
        }

        std::string xml_to_json(const pugi::xml_node &root)
        {
            spdlog::info("Converting XML to JSON");
            json j;
            for (auto &child : root.children())
            {
                if (child.attribute("type"))
                {
                    // If the node has a type attribute, convert it accordingly.
                    std::string type = child.attribute("type").as_string();
                    if (type == "array")
                    {
                        // Arrays require special handling.
                        json array = json::array();
                        for (auto &grandchild : child.children())
                        {
                            json obj;
                            for (auto &attr : grandchild.attributes())
                            {
                                obj[attr.name()] = attr.as_string();
                            }
                            obj["value"] = grandchild.text().get();
                            array.push_back(obj);
                        }
                        j[child.name()] = array;
                    }
                    else
                    {
                        // For other types, simply parse the attribute values.
                        for (auto &attr : child.attributes())
                        {
                            j[child.name()][attr.name()] = attr.as_string();
                        }
                        j[child.name()]["value"] = child.text().get();
                    }
                }
                else if (child.empty())
                {
                    // If the node is empty, only save its name.
                    j[child.name()] = nullptr;
                }
                else
                {
                    // Recursively process child nodes.
                    j[child.name()] = xml_to_json(child);
                }
            }
            return j.dump();
        }

    } // namespace XML
} // namespace OpenAPT