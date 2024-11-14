#ifndef LITHIUM_INDISERVER_COLLECTION_HPP
#define LITHIUM_INDISERVER_COLLECTION_HPP

#include "container.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace tinyxml2 {
class XMLElement;
}

/**
 * @class INDIDriverCollection
 * @brief A class to manage and parse INDI driver collections.
 *
 * This class provides functionality to collect, parse, and manage INDI drivers
 * from XML files and JSON configurations.
 */
class INDIDriverCollection {
public:
    /**
     * @brief Parses drivers from the specified path.
     * @param path The directory path containing the driver XML files.
     * @return True if drivers were successfully parsed, false otherwise.
     */
    auto parseDrivers(const std::string& path) -> bool;

    /**
     * @brief Parses a single device from an XML element.
     * @param device The XML element representing the device.
     * @param family The family attribute of the device group.
     * @return A shared pointer to the parsed INDIDeviceContainer, or nullptr on
     * failure.
     */
    auto parseDevice(tinyxml2::XMLElement* device, const char* family)
        -> std::shared_ptr<INDIDeviceContainer>;

    /**
     * @brief Collects XML files from the specified path.
     * @param path The directory path to search for XML files.
     * @return True if XML files were found, false otherwise.
     */
    auto collectXMLFiles(const std::string& path) -> bool;

    /**
     * @brief Parses custom drivers from a JSON configuration.
     * @param drivers The JSON array containing custom driver configurations.
     * @return True if custom drivers were successfully parsed, false otherwise.
     */
    auto parseCustomDrivers(const json& drivers) -> bool;

    /**
     * @brief Clears all custom drivers from the collection.
     */
    void clearCustomDrivers();

    /**
     * @brief Gets a driver by its label.
     * @param label The label of the driver to retrieve.
     * @return A shared pointer to the INDIDeviceContainer with the specified
     * label, or nullptr if not found.
     */
    auto getByLabel(const std::string& label)
        -> std::shared_ptr<INDIDeviceContainer>;

    /**
     * @brief Gets a driver by its name.
     * @param name The name of the driver to retrieve.
     * @return A shared pointer to the INDIDeviceContainer with the specified
     * name, or nullptr if not found.
     */
    auto getByName(const std::string& name)
        -> std::shared_ptr<INDIDeviceContainer>;

    /**
     * @brief Gets a driver by its binary path.
     * @param binary The binary path of the driver to retrieve.
     * @return A shared pointer to the INDIDeviceContainer with the specified
     * binary path, or nullptr if not found.
     */
    auto getByBinary(const std::string& binary)
        -> std::shared_ptr<INDIDeviceContainer>;

    /**
     * @brief Gets all driver families and their associated driver labels.
     * @return An unordered map where the key is the family name and the value
     * is a vector of driver labels.
     */
    auto getFamilies()
        -> std::unordered_map<std::string, std::vector<std::string>>;

private:
    std::string
        path_;  ///< The path to the directory containing driver XML files.
    std::vector<std::string> files_;  ///< A list of collected XML file paths.
    std::vector<std::shared_ptr<INDIDeviceContainer>>
        drivers_;  ///< A list of parsed drivers.
};

#endif  // LITHIUM_INDISERVER_COLLECTION_HPP