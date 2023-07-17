#include "xml.hpp"

namespace Lithium::XML
{
    px::xml_node read_xml(const std::string &filename)
    {
        px::xml_document doc;
        if (!doc.load_file(filename.c_str()))
        {
            // spdlog::error("Failed to read XML file {}", filename);
            return pugi::xml_node();
        }

        return doc.child("root");
    }

    bool modify_node(px::xml_node &root, const std::string &path, const std::string &value)
    {
        auto nodes = root.select_nodes(path.c_str());
        if (nodes.empty())
        {
            // spdlog::error("Failed to find node with path {}", path);
            return false;
        }

        for (auto &node : nodes)
        {
            node.node().text().set(value.c_str());
        }

        return true;
    }

    bool write_xml(const std::string &filename, const px::xml_node &root)
    {
        px::xml_document doc;
        auto newRoot = doc.append_copy(root);
        if (!doc.save_file(filename.c_str()))
        {
            // spdlog::error("Failed to write XML file {}", filename);
            return false;
        }

        return true;
    }

    bool validate_xml(const std::string &filename)
    {
        px::xml_document doc;
        if (!doc.load_file(filename.c_str()))
        {
            // spdlog::error("Failed to validate XML file {}", filename);
            return false;
        }

        return true;
    }

    json node_to_json(const px::xml_node &node)
    {
        json j;

        auto typeAttr = node.attribute("type");
        if (typeAttr)
        {
            std::string type = typeAttr.as_string();
            if (type == "array")
            {
                json array = json::array();
                for (const auto &child : node.children())
                {
                    json obj;
                    for (const auto &attr : child.attributes())
                    {
                        obj[attr.name()] = attr.value();
                    }
                    obj["value"] = child.child_value();
                    array.push_back(obj);
                }
                j[node.name()] = array;
            }
            else
            {
                for (const auto &attr : node.attributes())
                {
                    j[node.name()][attr.name()] = attr.value();
                }
                j[node.name()]["value"] = node.child_value();
            }
        }
        else if (node.empty())
        {
            j[node.name()] = nullptr;
        }
        else
        {
            for (const auto &child : node.children())
            {
                j[node.name()] = node_to_json(child);
            }
        }

        return j;
    }

    std::string xml_to_json(const px::xml_node &root)
    {
        json j;
        j = node_to_json(root);

        return j.dump();
    }
} // namespace Lithium::XML
