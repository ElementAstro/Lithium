#include "indiserver.hpp"

#include "atom/system/software.hpp"

INDIManager::INDIManager(std::unique_ptr<Connector> conn)
    : connector(std::move(conn)) {}

INDIManager::~INDIManager() = default;

auto INDIManager::startServer() -> bool { return connector->startServer(); }

auto INDIManager::stopServer() -> bool { return connector->stopServer(); }

auto INDIManager::isRunning() -> bool { return connector->isRunning(); }

auto INDIManager::isInstalled() -> bool {
    return atom::system::checkSoftwareInstalled("hydrogenserver");
}

auto INDIManager::startDriver(
    const std::shared_ptr<INDIDeviceContainer>& driver) -> bool {
    return connector->startDriver(driver);
}

auto INDIManager::stopDriver(
    const std::shared_ptr<INDIDeviceContainer>& driver) -> bool {
    return connector->stopDriver(driver);
}

auto INDIManager::setProp(const std::string& dev, const std::string& prop,
                          const std::string& element,
                          const std::string& value) -> bool {
    return connector->setProp(dev, prop, element, value);
}

auto INDIManager::getProp(const std::string& dev,
                                 const std::string& prop,
                                 const std::string& element) -> std::string {
    return connector->getProp(dev, prop, element);
}

std::string INDIManager::getState(const std::string& dev,
                                  const std::string& prop) {
    return connector->getState(dev, prop);
}

std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
INDIManager::getRunningDrivers() {
    return connector->getRunningDrivers();
}
