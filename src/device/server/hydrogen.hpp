/*
 * hydrogen.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Hydrogen Device Manager

**************************************************/

#ifndef LITHIUM_DEVICE_HYDROGEN_HPP
#define LITHIUM_DEVICE_HYDROGEN_HPP

#include "config.h"

#include "connector.hpp"

#include <memory>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include "hydrogen_driver.hpp"

namespace lithium {

class HydrogenManager : public BasicManager {
public:
    /**
     * @brief 构造函数
     * @param hst Hydrogen服务器的主机名，默认为"localhost"
     * @param prt Hydrogen服务器的端口号，默认为7624
     * @param cfg Hydrogen配置文件路径，默认为空字符串
     * @param dta Hydrogen驱动程序路径，默认为"/usr/share/Hydrogen"
     * @param fif Hydrogen FIFO路径，默认为"/tmp/HydrogenFIFO"
     */
    explicit HydrogenManager(const std::string &hst = "localhost",
                             int prt = 7624, const std::string &cfg = "",
                             const std::string &dta = "/usr/share/hydrogen",
                             const std::string &fif = "/tmp/hydrogenFIFO");

    virtual ~HydrogenManager();

    static std::shared_ptr<HydrogenManager> createShared(const std::string &hst = "localhost",
                                             int prt = 7624, const std::string &cfg = "",
                                             const std::string &dta = "/usr/share/hydrogen",
                                             const std::string &fif = "/tmp/hydrogenFIFO");
                                             
    static std::unique_ptr<HydrogenManager> createUnique(const std::string &hst = "localhost",
                                             int prt = 7624, const std::string &cfg = "",
                                             const std::string &dta = "/usr/share/hydrogen",
                                             const std::string &fif = "/tmp/hydrogenFIFO");

    /**
     * @brief 启动Hydrogen服务器
     */
    virtual bool startServer() override;

    /**
     * @brief 停止Hydrogen服务器
     */
    virtual bool stopServer() override;

    /**
     * @brief 检查Hydrogen服务器是否正在运行
     * @return 如果Hydrogen服务器正在运行，则返回true；否则返回false
     */
    virtual bool isRunning() override;

    /**
     * @brief 检查Hydrogen驱动程序是否已安装
     * @return 如果Hydrogen驱动程序已安装，则返回true；否则返回false
     */
    virtual bool isInstalled() override;

    /**
     * @brief 启动Hydrogen驱动程序
     * @param driver 要启动的Hydrogen驱动程序的HydrogenDeviceContainer对象
     */
    bool startDriver(std::shared_ptr<HydrogenDeviceContainer> driver);

    /**
     * @brief 停止Hydrogen驱动程序
     * @param driver 要停止的Hydrogen驱动程序的HydrogenDeviceContainer对象
     */
    bool stopDriver(std::shared_ptr<HydrogenDeviceContainer> driver);

    /**
     * @brief 设置设备属性值
     * @param dev 设备名称
     * @param prop 属性名称
     * @param element 元素名称
     * @param value 要设置的属性值
     */
    bool setProp(const std::string &dev, const std::string &prop,
                 const std::string &element, const std::string &value);

    /**
     * @brief 获取设备属性值
     * @param dev 设备名称
     * @param prop 属性名称
     * @param element 元素名称
     * @return 获取到的属性值
     */
    std::string getProp(const std::string &dev, const std::string &prop,
                        const std::string &element);

    /**
     * @brief 获取设备状态
     * @param dev 设备名称
     * @param prop 属性名称
     * @return 获取到的设备状态
     */
    std::string getState(const std::string &dev, const std::string &prop);

    /**
     * @brief 获取正在运行的驱动程序列表
     * @return
     * 包含正在运行的驱动程序的映射表，键为驱动程序名称，值为HydrogenDeviceContainer对象
     */
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<HydrogenDeviceContainer>>
    getRunningDrivers();
#else
    std::unordered_map<std::string, std::shared_ptr<HydrogenDeviceContainer>>
    getRunningDrivers();
#endif

    /**
     * @brief 获取设备列表
     * @return
     * 包含设备信息的向量，每个元素是一个映射表，包含设备名称和设备类型等信息
     */
#if ENABLE_FASTHASH
    static std::vector<emhash8::HashMap<std::string, std::string>> getDevices();
#else
    static std::vector<std::unordered_map<std::string, std::string>>
    getDevices();
#endif

protected:
    json _startServer(const json &params);

private:
    std::string host;         ///< Hydrogen服务器的主机名
    int port;                 ///< Hydrogen服务器的端口号
    std::string config_path;  ///< Hydrogen配置文件路径
    std::string data_path;    ///< Hydrogen驱动程序路径
    std::string fifo_path;    ///< Hydrogen FIFO路径
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<HydrogenDeviceContainer>>
        running_drivers;
#else
    std::unordered_map<std::string, std::shared_ptr<HydrogenDeviceContainer>>
        running_drivers;  ///< 正在运行的驱动程序列表
#endif
};
}  // namespace lithium

#endif
