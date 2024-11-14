// indihub_agent.hpp
#pragma once

#include <memory>
#include <string>
#include <thread>

class AsyncSystemCommand;

static const std::string INDIHUB_AGENT_DEFAULT_MODE = "local";
static const std::string INDIHUB_AGENT_CONFIG = "/tmp/indihub_agent.conf";

/**
 * @class IndiHubAgent
 * @brief A class to manage the INDIHub agent.
 *
 * This class provides functionality to start and stop the INDIHub agent,
 * check its running status, and manage its configuration.
 */
class IndiHubAgent {
public:
    /**
     * @brief Constructs an IndiHubAgent with the given parameters.
     * @param web_addr The web address of the INDIHub server.
     * @param hostname The hostname of the INDIHub agent.
     * @param port The port number of the INDIHub agent.
     */
    IndiHubAgent(const std::string& web_addr, const std::string& hostname,
                 int port);

    /**
     * @brief Destructor for IndiHubAgent.
     */
    ~IndiHubAgent();

    /**
     * @brief Starts the INDIHub agent with the given profile and mode.
     * @param profile The profile to use for the INDIHub agent.
     * @param mode The mode to run the INDIHub agent in (default is "local").
     * @param conf The path to the configuration file (default is
     * "/tmp/indihub_agent.conf").
     */
    void start(const std::string& profile,
               const std::string& mode = INDIHUB_AGENT_DEFAULT_MODE,
               const std::string& conf = INDIHUB_AGENT_CONFIG);

    /**
     * @brief Stops the INDIHub agent.
     */
    void stop();

    /**
     * @brief Checks if the INDIHub agent is currently running.
     * @return True if the agent is running, false otherwise.
     */
    bool isRunning() const;

    /**
     * @brief Gets the current mode of the INDIHub agent.
     * @return The current mode of the agent.
     */
    std::string getMode() const;

private:
    /**
     * @brief Runs the INDIHub agent with the given profile, mode, and
     * configuration.
     * @param profile The profile to use for the INDIHub agent.
     * @param mode The mode to run the INDIHub agent in.
     * @param conf The path to the configuration file.
     */
    void run(const std::string& profile, const std::string& mode,
             const std::string& conf);

    std::string web_addr_;  ///< The web address of the INDIHub server.
    std::string hostname_;  ///< The hostname of the INDIHub agent.
    int port_;              ///< The port number of the INDIHub agent.
    std::string mode_;      ///< The current mode of the INDIHub agent.
    std::unique_ptr<AsyncSystemCommand>
        async_cmd_;  ///< The asynchronous system command for running the agent.
    std::unique_ptr<std::thread>
        command_thread_;  ///< The thread for running the asynchronous command.
};