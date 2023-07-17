#pragma once

#include <string>
#include <vector>
#include <map>

#include "json.hpp"
#include "pugixml/pugixml.hpp"

using json = nlohmann::json;
namespace px = pugi;

namespace Lithium::XML
{
    px::xml_node read_xml(const std::string &filename);
    bool modify_node(px::xml_node &root, const std::string &path, const std::string &value);
    bool write_xml(const std::string &filename, const px::xml_node &root);
    bool validate_xml(const std::string &filename);
    json node_to_json(const px::xml_node &node);
    std::string xml_to_json(const px::xml_node &root);
} // namespace Lithium::XML
