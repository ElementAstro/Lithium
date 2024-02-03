#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>

#include "json.hpp"
using json = nlohmann::json;

constexpr int API_VERSION = 1;

/**
 * @brief The Device class represents a device connected to a network.
 *
 * This class provides methods for interacting with the device, such as executing actions, sending commands,
 * and retrieving information.
 */
class Device
{
public:
    /**
     * @brief Constructs a Device object with the specified address, device type, device number, and protocol.
     *
     * @param address The address of the device.
     * @param device_type The type of the device.
     * @param device_number The number of the device.
     * @param protocol The communication protocol used to connect to the device.
     */
    Device(const std::string &address, const std::string &device_type, int device_number, const std::string &protocol);

    /**
     * @brief Executes an action on the device with the specified name and parameters.
     *
     * @param ActionName The name of the action to execute.
     * @param Parameters The parameters to pass to the action.
     * @return The result of the action execution.
     */
    std::string Action(const std::string &ActionName, const std::vector<std::string> &Parameters);

    /**
     * @brief Sends a command to the device without waiting for a response.
     *
     * @param Command The command to send.
     * @param Raw Indicates whether the command should be sent as raw text.
     */
    void CommandBlind(const std::string &Command, bool Raw);

    /**
     * @brief Sends a command to the device and retrieves a boolean response.
     *
     * @param Command The command to send.
     * @param Raw Indicates whether the command should be sent as raw text.
     * @return The boolean response from the device.
     */
    bool CommandBool(const std::string &Command, bool Raw);

    /**
     * @brief Sends a command to the device and retrieves a string response.
     *
     * @param Command The command to send.
     * @param Raw Indicates whether the command should be sent as raw text.
     * @return The string response from the device.
     */
    std::string CommandString(const std::string &Command, bool Raw);

    /**
     * @brief Returns the connection status of the device.
     *
     * @return True if the device is connected, false otherwise.
     */
    bool get_Connected() const;

    /**
     * @brief Sets the connection status of the device.
     *
     * @param ConnectedState The connection status to set.
     */
    void set_Connected(bool ConnectedState);

    /**
     * @brief Returns the description of the device.
     *
     * @return The description of the device.
     */
    std::string get_Description() const;

    /**
     * @brief Returns the driver information of the device.
     *
     * @return The driver information of the device.
     */
    std::vector<std::string> get_DriverInfo() const;

    /**
     * @brief Returns the driver version of the device.
     *
     * @return The driver version of the device.
     */
    std::string get_DriverVersion() const;

    /**
     * @brief Returns the interface version of the device.
     *
     * @return The interface version of the device.
     */
    int get_InterfaceVersion() const;

    /**
     * @brief Returns the name of the device.
     *
     * @return The name of the device.
     */
    std::string get_Name() const;

    /**
     * @brief Returns the supported actions of the device.
     *
     * @return The supported actions of the device.
     */
    std::vector<std::string> get_SupportedActions() const;

    /**
     * @brief Sends a GET request to the device with the specified attribute and data.
     *
     * @param attribute The attribute to retrieve.
     * @param data The additional data to include in the request.
     * @param tmo The timeout for the request.
     * @return The JSON response from the device.
     */
    json _get(const std::string &attribute, const std::map<std::string, std::string> &data, double tmo) const;

    /**
     * @brief Sends a PUT request to the device with the specified attribute and data.
     *
     * @param attribute The attribute to modify.
     * @param data The data to send in the request.
     * @param tmo The timeout for the request.
     * @return The JSON response from the device.
     */
    json _put(const std::string &attribute, const std::map<std::string, std::any> &data, double tmo) const;

private:
    /** The address of the device. */
    std::string address;

    /** The type of the device. */
    std::string device_type;

    /** The number of the device. */
    int device_number;

    /** The communication protocol used to connect to the device. */
    std::string protocol;

    /** The API version used by the device. */
    int api_version;

    /** The base URL for accessing the device's API. */
    std::string base_url;

    /** The client ID used for communication with the device. */
    static int _client_id;

    /** The client transaction ID used for communication with the device. */
    static int _client_trans_id;

    /** The mutex used for thread-safe access to the client transaction ID. */
    static std::mutex _ctid_lock;
};
