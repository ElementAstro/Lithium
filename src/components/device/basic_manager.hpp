#pragma once

#include <string>

class BasicManager
{
public:
    /**
     * @brief Constructor
     */
    BasicManager();

    /**
     * @brief Destructor
     */
    virtual ~BasicManager();

    /**
     * @brief Start the server
     */
    virtual bool startServer();

    /**
     * @brief Stop the server
     * @return If the server is stopped, true, otherwise false.
     */
    virtual bool stopServer();

    /**
     * @brief Checks if the server is running.
     * @return If the server is running, true, otherwise false.
     */
    virtual bool isRunning();

    /**
     * @brief Checks if the server is installed.
     * @return If the server is installed, true, otherwise false.
     */
    virtual bool isInstalled();
};
