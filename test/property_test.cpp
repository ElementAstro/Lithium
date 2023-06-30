#include "../src/property/imessage.hpp"

using namespace OpenAPT::Property;

int main()
{
    IImage image;
    image.width = 800;
    image.height = 600;
    image.duration = 10.5;
    image.setValue("aaaaa");

    std::cout << "JSON: " << image.toJson() << std::endl;
    std::cout << "XML: " << image.toXml() << std::endl;

    try
    {
        bool value = image.getValue<std::string>();
        std::cout << "Value: " << value << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}