#ifndef LITHIUM_ADDON_TEMPLATE_CONNECTOR_HPP
#define LITHIUM_ADDON_TEMPLATE_CONNECTOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Connector {
public:
    virtual ~Connector() = default;
    virtual auto startServer() -> bool = 0;
    virtual auto stopServer() -> bool = 0;
    virtual auto isRunning() -> bool = 0;
    virtual auto startDriver(
        const std::shared_ptr<class INDIDeviceContainer>& driver) -> bool = 0;
    virtual auto stopDriver(
        const std::shared_ptr<class INDIDeviceContainer>& driver) -> bool = 0;
    virtual auto setProp(const std::string& dev, const std::string& prop,
                         const std::string& element,
                         const std::string& value) -> bool = 0;
    virtual auto getProp(const std::string& dev, const std::string& prop,
                         const std::string& element) -> std::string = 0;
    virtual auto getState(const std::string& dev,
                          const std::string& prop) -> std::string = 0;
    virtual auto getRunningDrivers()
        -> std::unordered_map<std::string,
                              std::shared_ptr<class INDIDeviceContainer>> = 0;
    virtual auto getDevices()
        -> std::vector<std::unordered_map<std::string, std::string>> = 0;
};

#endif  // LITHIUM_ADDON_TEMPLATE_CONNECTOR_HPP
