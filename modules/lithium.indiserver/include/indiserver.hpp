#ifndef LITHIUM_INDISERVER_HPP
#define LITHIUM_INDISERVER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include "addon/template/connector.hpp"

/**
 * @class INDIManager
 * @brief A class to manage the INDI server and its drivers.
 *
 * This class provides functionality to start and stop the INDI server, manage
 * drivers, and set or get properties of INDI devices.
 */
class INDIManager {
public:
    /**
     * @brief Constructs an INDIManager with the given connector.
     * @param connector A unique pointer to a Connector object.
     */
    explicit INDIManager(std::unique_ptr<Connector> connector);

    /**
     * @brief Destructor for INDIManager.
     */
    ~INDIManager();

    /**
     * @brief Starts the INDI server.
     * @return True if the server was started successfully, false otherwise.
     */
    bool startServer();

    /**
     * @brief Stops the INDI server.
     * @return True if the server was stopped successfully, false otherwise.
     */
    bool stopServer();

    /**
     * @brief Checks if the INDI server is running.
     * @return True if the server is running, false otherwise.
     */
    bool isRunning();

    /**
     * @brief Checks if the INDI server software is installed.
     * @return True if the software is installed, false otherwise.
     */
    bool isInstalled();

    /**
     * @brief Starts an INDI driver.
     * @param driver A shared pointer to the INDIDeviceContainer representing
     * the driver.
     * @return True if the driver was started successfully, false otherwise.
     */
    bool startDriver(const std::shared_ptr<INDIDeviceContainer>& driver);

    /**
     * @brief Stops an INDI driver.
     * @param driver A shared pointer to the INDIDeviceContainer representing
     * the driver.
     * @return True if the driver was stopped successfully, false otherwise.
     */
    bool stopDriver(const std::shared_ptr<INDIDeviceContainer>& driver);

    /**
     * @brief Sets a property of an INDI device.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @param element The name of the element.
     * @param value The value to set.
     * @return True if the property was set successfully, false otherwise.
     */
    bool setProp(const std::string& dev, const std::string& prop,
                 const std::string& element, const std::string& value);

    /**
     * @brief Gets a property of an INDI device.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @param element The name of the element.
     * @return The value of the property.
     */
    std::string getProp(const std::string& dev, const std::string& prop,
                        const std::string& element);

    /**
     * @brief Gets the state of an INDI device property.
     * @param dev The name of the device.
     * @param prop The name of the property.
     * @return The state of the property.
     */
    std::string getState(const std::string& dev, const std::string& prop);

    /**
     * @brief Gets a list of running INDI drivers.
     * @return An unordered map where the key is the driver label and the value
     * is a shared pointer to the INDIDeviceContainer.
     */
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
    getRunningDrivers();

private:
    std::unique_ptr<Connector> connector;  ///< The connector used to manage the
                                           ///< INDI server and drivers.
};

#endif  // LITHIUM_INDISERVER_HPP