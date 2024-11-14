#ifndef LITHIUM_INDISERVER_CONTAINER_HPP
#define LITHIUM_INDISERVER_CONTAINER_HPP

#include <string>
#include <utility>

/**
 * @class INDIDeviceContainer
 * @brief A container class for INDI device information.
 *
 * This class holds information about an INDI device, including its name, label,
 * version, binary path, family, skeleton path, and whether it is a custom
 * device.
 */
class INDIDeviceContainer {
public:
    std::string name;      ///< The name of the device.
    std::string label;     ///< The label of the device.
    std::string version;   ///< The version of the device.
    std::string binary;    ///< The binary path of the device.
    std::string family;    ///< The family to which the device belongs.
    std::string skeleton;  ///< The skeleton path of the device (optional).
    bool custom;           ///< Indicates whether the device is custom.

    /**
     * @brief Constructs an INDIDeviceContainer with the given parameters.
     * @param name The name of the device.
     * @param label The label of the device.
     * @param version The version of the device.
     * @param binary The binary path of the device.
     * @param family The family to which the device belongs.
     * @param skeleton The skeleton path of the device (optional).
     * @param custom Indicates whether the device is custom (default is false).
     */
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