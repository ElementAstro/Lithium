#ifndef LITHIUM_INDISERVER_CONTAINER_HPP
#define LITHIUM_INDISERVER_CONTAINER_HPP

#include <string>
#include <utility>

class INDIDeviceContainer {
public:
    std::string name;
    std::string label;
    std::string version;
    std::string binary;
    std::string family;
    std::string skeleton;
    bool custom;

    INDIDeviceContainer(std::string name, std::string label,
                        std::string version, std::string binary,
                        std::string family, std::string skeleton = "",
                        bool custom = false)
        : name(std::move(name)),
          label(std::move(label)),
          version(std::move(version)),
          binary(std::move(binary)),
          family(std::move(family)),
          skeleton(std::move(skeleton)),
          custom(custom) {}
};

#endif  // LITHIUM_INDISERVER_CONTAINER_HPP
