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

namespace OpenAPT::XML {
    
    /**
     * @brief 读取 XML 文件并返回根节点
     *
     * @param filename XML 文件路径
     * @return pugi::xml_node 根节点指针
     */
    pugi::xml_node read_xml(const char* filename) {
        spdlog::info("Reading XML file {}", filename);
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filename);
        if (!result) {
            spdlog::error("Failed to read XML file {}: {}", filename, result.description());
            return pugi::xml_node();
        }
        return doc.child("root");
    }

    /**
     * @brief 根据路径查找节点并修改其值
     *
     * @param root 根节点
     * @param path 节点路径
     * @param value 新值
     * @return true 修改成功
     * @return false 修改失败
     */
    bool modify_node(pugi::xml_node& root, const char* path, const char* value) {
        spdlog::info("Modifying XML node with path {} to value {}", path, value);
        pugi::xpath_node_set nodes = root.select_nodes(path);
        if (nodes.empty()) {
            spdlog::error("Failed to find node with path {}", path);
            return false;
        }

        for (auto& node : nodes) {
            node.node().text().set(value);
        }
        return true;
    }

    /**
     * @brief 存储 XML 文件
     *
     * @param filename XML 文件路径
     * @param root 根节点
     * @return true 存储成功
     * @return false 存储失败
     */
    bool write_xml(const char* filename, const pugi::xml_node& root) {
        spdlog::info("Writing XML file {}", filename);
        pugi::xml_document doc;
        doc.append_child("root").append_copy(root);

        if (!doc.save_file(filename)) {
            spdlog::error("Failed to write XML file {}", filename);
            return false;
        }
        return true;
    }

    /**
     * @brief 校验 XML 文件是否合法
     *
     * @param filename XML 文件路径
     * @return true 合法
     * @return false 不合法
     */
    bool validate_xml(const char* filename) {
        spdlog::info("Validating XML file {}", filename);
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filename);
        if (!result) {
            spdlog::error("Failed to validate XML file {}: {}", filename, result.description());
            return false;
        }
        return true;
    }

    /**
     * @brief 将 XML 转换为 JSON 字符串
     *
     * @param root 根节点
     * @return std::string 转换后的 JSON 字符串
     */
    std::string xml_to_json(const pugi::xml_node& root) {
        spdlog::info("Converting XML to JSON");
        json j;
        for (auto& child : root.children()) {
            if (child.attribute("type")) {
                // 如果节点有 type 属性，则按照该属性值来转换
                std::string type = child.attribute("type").as_string();
                if (type == "array") {
                    // 数组类型的节点需要特殊处理
                    json array = json::array();
                    for (auto& grandchild : child.children()) {
                        json obj;
                        for (auto& attr : grandchild.attributes()) {
                            obj[attr.name()] = attr.as_string();
                        }
                        obj["value"] = grandchild.text().get();
                        array.push_back(obj);
                    }
                    j[child.name()] = array;
                } else {
                    // 其他类型直接按照属性值来解析
                    for (auto& attr : child.attributes()) {
                        j[child.name()][attr.name()] = attr.as_string();
                    }
                    j[child.name()]["value"] = child.text().get();
                }
            } else if (child.empty()) {
                // 如果是空节点，则只保存节点名
                j[child.name()] = nullptr;
            } else {
                // 否则递归处理子节点
                j[child.name()] = xml_to_json(child);
            }
        }
        return j.dump();
    }

}
