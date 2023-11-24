/*
 * indidevice_manager.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: INDI Device Manager

**************************************************/


#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>

class INDIDeviceContainer;

class INDIManager
{
public:
    /**
     * @brief 构造函数
     * @param hst INDI服务器的主机名，默认为"localhost"
     * @param prt INDI服务器的端口号，默认为7624
     * @param cfg INDI配置文件路径，默认为空字符串
     * @param dta INDI驱动程序路径，默认为"/usr/share/indi"
     * @param fif INDI FIFO路径，默认为"/tmp/indiFIFO"
     */
    INDIManager(const std::string &hst = "localhost", int prt = 7624, const std::string &cfg = "", const std::string &dta = "/usr/share/indi", const std::string &fif = "/tmp/indiFIFO");

    /**
     * @brief 启动INDI服务器
     */
    void start_server();

    /**
     * @brief 停止INDI服务器
     */
    void stop_server();

    /**
     * @brief 检查INDI服务器是否正在运行
     * @return 如果INDI服务器正在运行，则返回true；否则返回false
     */
    bool is_running();

    /**
     * @brief 启动INDI驱动程序
     * @param driver 要启动的INDI驱动程序的INDIDeviceContainer对象
     */
    void start_driver(std::shared_ptr<INDIDeviceContainer> driver);

    /**
     * @brief 停止INDI驱动程序
     * @param driver 要停止的INDI驱动程序的INDIDeviceContainer对象
     */
    void stop_driver(std::shared_ptr<INDIDeviceContainer> driver);

    /**
     * @brief 设置设备属性值
     * @param dev 设备名称
     * @param prop 属性名称
     * @param element 元素名称
     * @param value 要设置的属性值
     */
    void set_prop(const std::string &dev, const std::string &prop, const std::string &element, const std::string &value);

    /**
     * @brief 获取设备属性值
     * @param dev 设备名称
     * @param prop 属性名称
     * @param element 元素名称
     * @return 获取到的属性值
     */
    std::string get_prop(const std::string &dev, const std::string &prop, const std::string &element);

    /**
     * @brief 获取设备状态
     * @param dev 设备名称
     * @param prop 属性名称
     * @return 获取到的设备状态
     */
    std::string get_state(const std::string &dev, const std::string &prop);

    /**
     * @brief 获取正在运行的驱动程序列表
     * @return 包含正在运行的驱动程序的映射表，键为驱动程序名称，值为INDIDeviceContainer对象
     */
    std::map<std::string, std::shared_ptr<INDIDeviceContainer>> get_running_drivers();

    /**
     * @brief 获取设备列表
     * @return 包含设备信息的向量，每个元素是一个映射表，包含设备名称和设备类型等信息
     */
    static std::vector<std::map<std::string, std::string>> get_devices();

private:
    std::string host;                                                            ///< INDI服务器的主机名
    int port;                                                                    ///< INDI服务器的端口号
    std::string config_path;                                                     ///< INDI配置文件路径
    std::string data_path;                                                       ///< INDI驱动程序路径
    std::string fifo_path;                                                       ///< INDI FIFO路径
    std::map<std::string, std::shared_ptr<INDIDeviceContainer>> running_drivers; ///< 正在运行的驱动程序列表
};
