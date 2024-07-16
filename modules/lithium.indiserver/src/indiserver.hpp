#ifndef LITHIUM_INDISERVER_HPP
#define LITHIUM_INDISERVER_HPP

#include "addon/template/connector.hpp"

class INDIManager {
public:
    explicit INDIManager(std::unique_ptr<Connector> connector);

    ~INDIManager();

    bool startServer();
    bool stopServer();
    bool isRunning();
    bool isInstalled();
    bool startDriver(const std::shared_ptr<INDIDeviceContainer>& driver);
    bool stopDriver(const std::shared_ptr<INDIDeviceContainer>& driver);
    bool setProp(const std::string& dev, const std::string& prop, const std::string& element, const std::string& value);
    std::string getProp(const std::string& dev, const std::string& prop, const std::string& element);
    std::string getState(const std::string& dev, const std::string& prop);
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>> getRunningDrivers();

private:
    std::unique_ptr<Connector> connector;
};

#endif // LITHIUM_INDISERVER_HPP
