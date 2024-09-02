#ifndef LITHIUM_INDISERVER_CONNECTOR_HPP
#define LITHIUM_INDISERVER_CONNECTOR_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "addon/template/connector.hpp"

class INDIConnector : public Connector {
public:
    INDIConnector(const std::string& hst = "localhost", int prt = 7624, const std::string& cfg = "",
                  const std::string& dta = "/usr/share/indi", const std::string& fif = "/tmp/indi.fifo");
    ~INDIConnector() override = default;
    auto startServer() -> bool override;
    auto stopServer() -> bool override;
    auto isRunning() -> bool override;
    auto isInstalled() -> bool;
    auto startDriver(const std::shared_ptr<class INDIDeviceContainer>& driver)
        -> bool override;
    auto stopDriver(const std::shared_ptr<class INDIDeviceContainer>& driver)
        -> bool override;
    auto setProp(const std::string& dev, const std::string& prop,
                 const std::string& element,
                 const std::string& value) -> bool override;
    auto getProp(const std::string& dev, const std::string& prop,
                 const std::string& element) -> std::string override;
    auto getState(const std::string& dev,
                  const std::string& prop) -> std::string override;
    auto getRunningDrivers()
        -> std::unordered_map<
            std::string, std::shared_ptr<class INDIDeviceContainer>> override;
    auto getDevices() -> std::vector<std::unordered_map<std::string, std::string>> override;
private:
    std::string host_;         ///< INDI服务器的主机名
    int port_;                 ///< INDI服务器的端口号
    std::string config_path_;  ///< INDI配置文件路径
    std::string data_path_;    ///< INDI驱动程序路径
    std::string fifo_path_;    ///< INDI FIFO路径
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<INDIDeviceContainer>>
        running_drivers_;
#else
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
        running_drivers_;  ///< 正在运行的驱动程序列表
#endif
};

#endif  // LITHIUM_INDISERVER_CONNECTOR_HPP
