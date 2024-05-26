#ifndef LITHIUM_INDISERVER_CONTAINER_HPP
#define LITHIUM_INDISERVER_CONTAINER_HPP

#include <string>

class INDIDeviceContainer
{
public:
    std::string name;
    std::string label;
    std::string version;
    std::string binary;
    std::string family;
    std::string skeleton;
    bool custom;

    explicit INDIDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                        const std::string &binary, const std::string &family,
                        const std::string &skeleton = "", bool custom = false);
};

#endif
