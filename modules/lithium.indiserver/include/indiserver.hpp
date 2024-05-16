#ifndef LITHIUM_INDISERVER_HPP
#define LITHIUM_INDISERVER_HPP

#include "collection.hpp"

class INDIManager {
public:
    explicit INDIManager(const std::string &hst = "localhost", int prt = 7624,
                         const std::string &cfg = "",
                         const std::string &dta = "/usr/share/hydrogen",
                         const std::string &fif = "/tmp/hydrogenFIFO");

    ~INDIManager();

    static std::shared_ptr<INDIManager> createShared(
        const std::string &hst = "localhost", int prt = 7624,
        const std::string &cfg = "",
        const std::string &dta = "/usr/share/hydrogen",
        const std::string &fif = "/tmp/hydrogenFIFO");

    bool startServer();

    bool stopServer();

    bool isRunning();

    bool isInstalled();

    bool startDriver(std::shared_ptr<INDIDeviceContainer> driver);

    bool stopDriver(std::shared_ptr<INDIDeviceContainer> driver);

    bool setProp(const std::string &dev, const std::string &prop,
                 const std::string &element, const std::string &value);

    std::string getProp(const std::string &dev, const std::string &prop,
                        const std::string &element);

    std::string getState(const std::string &dev, const std::string &prop);

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<INDIDeviceContainer>>
    getRunningDrivers();
#else
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
    getRunningDrivers();
#endif

#if ENABLE_FASTHASH
    static std::vector<emhash8::HashMap<std::string, std::string>> getDevices();
#else
    static std::vector<std::unordered_map<std::string, std::string>>
    getDevices();
#endif

private:
    std::string host;         ///< INDI服务器的主机名
    int port;                 ///< INDI服务器的端口号
    std::string config_path;  ///< INDI配置文件路径
    std::string data_path;    ///< INDI驱动程序路径
    std::string fifo_path;    ///< INDI FIFO路径
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<INDIDeviceContainer>>
        running_drivers;
#else
    std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
        running_drivers;  ///< 正在运行的驱动程序列表
#endif
};

#endif  // LITHIUM_INDISERVER_HPP
