#ifndef LITHIUM_INDISERVER_CONNECTOR_HPP
#define LITHIUM_INDISERVER_CONNECTOR_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "addon/template/connector.hpp"

/**
 * @class INDIConnector
 * @brief A class to manage the connection and interaction with an INDI server.
 *
 * This class provides functionality to start and stop the INDI server, manage drivers,
 * and set or get properties of INDI devices.
 */
class INDIConnector : public Connector {
public:
    /**
     * @brief Constructs an INDIConnector with the given parameters.
     * @param hst The hostname of the INDI server (default is "localhost").
     * @param prt The port number of the INDI server (default is 7624).
     * @param cfg The path to the INDI configuration files (default is an empty string).
     * @param dta The path to the INDI data files (default is "/usr/share/indi").
     * @param fif The path to the INDI FIFO file (default is "/tmp/indi.fifo").
     */
    INDIConnector(const std::string& hst = "localhost", int prt = 7624, const std::string& cfg = "",
                  const std::string& dta = "/usr/share/indi", const std::string& fif = "/tmp/indi.fifo");

    /**
     * @brief Destructor for INDIConnector.
     */
    ~INDIConnector() override = default;

    /**
     * @brief Starts the INDI server.
     * @return True if the server was started successfully, false otherwise.
     */
    auto startServer() -> bool override;

    /**
     * @brief Stops the INDI server.
     * @return True if the server was stopped successfully, false otherwise.
     */
    auto stopServer() -> bool override;

    /**
     * @brief Checks if the INDI server is running.
     * @return True if the server is running, false otherwise.
     */
    auto isRunning() -> bool override;

    /**
     * @brief Checks if the INDI server software is installed.
     * @return True if the software is installed, false otherwise.
     */
    auto isInstalled() -> bool;

    /**
     * @brief Starts an INDI driver.
     * @param driver A shared pointer to the INDIDeviceContainer representing the driver.
     * @return True if the driver was started successfully, false otherwise.
     */
    auto startDriver(const std::shared_ptr<class INDIDeviceContainer>& driver) -> bool override;

    /**
     * @brief Stops an INDI driver.
     * @param driver A shared pointer to the INDIDeviceContainer representing the driver.
     * @return True if the driver was stopped successfully, false otherwise.
     */
    auto stopDriver(const std::shared_ptr<class INDIDeviceContainer>& driver) -> bool override;

    /**
     * @brief Sets a property of an INDI device.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @param element The name of the element.
     * @param value The value to set.
     * @return True if the property was set successfully, false otherwise.
     */
    auto setProp(const std::string& dev, const std::string& prop, const std::string& element, const std::string& value) -> bool override;

    /**
     * @brief Gets a property of an INDI device.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @param element The name of the element.
     * @return The value of the property.
     */
    auto getProp(const std::string& dev, const std::string& prop, const std::string& element) -> std::string override;

    /**
     * @brief Gets the state of an INDI device property.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @return The state of the property.
     */
    auto getState(const std::string& dev, const std::string& prop) -> std::string override;

    /**
     * @brief Gets a list of running INDI drivers.
     * @return An unordered map where the key is the driver label and the value is a shared pointer to the INDIDeviceContainer.
     */
    auto getRunningDrivers() -> std::unordered_map<std::string, std::shared_ptr<class INDIDeviceContainer>> override;

    /**
     * @brief Gets a list of INDI devices.
     * @return A vector of unordered maps, each representing a device with its properties.
     */
    auto getDevices() -> std::vector<std::unordered_map<std::string, std::string>> override;

private:
    /**
     * @brief Validates the paths for configuration and data files.
     */
    void validatePaths();

    std::string host_;         ///< The hostname of the INDI server.
    int port_;                 ///< The port number of the INDI server.
    std::string config_path_;  ///< The path to the INDI configuration files.
    std::string data_path_;    ///< The path to the INDI data files.
    std::string fifo_path_;    ///< The path to the INDI FIFO file.

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<INDIDeviceContainer>> running_drivers_;
#else
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>> running_drivers_; ///< A list of running drivers.
#endif
};

#endif  // LITHIUM_INDISERVER_CONNECTOR_HPP