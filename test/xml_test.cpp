#include <iostream>
#include <fstream>

#include "../src/property/json.hpp"
#include "../src/property/xml.hpp"

using json = nlohmann::json;

int main()
{

    // 读取XML文件
    std::string xmlFilename = "data.xml";
    auto root = OpenAPT::XML::read_xml(xmlFilename);
    // 将修改后的XML写回文件
    std::string modifiedXmlFilename = "modified_data.xml";
    OpenAPT::XML::write_xml(modifiedXmlFilename, root);

    // 验证修改后的XML文件
    bool isXmlValid = OpenAPT::XML::validate_xml(modifiedXmlFilename);

    if (isXmlValid)
    {

        // 将修改后的XML转换为JSON
        std::string jsonStr = OpenAPT::XML::xml_to_json(root);
        std::cout << jsonStr;
    }
    else
    {
    }

    return 0;
}
