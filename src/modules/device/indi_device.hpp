/*
 * idriver.hpp
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

Description: INDI Web Driver

**************************************************/

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class INDIDeviceContainer
{
public:
    std::string name;
    std::string label;
    std::string version;
    std::string binary;
    std::string family;
    std::string skeleton;
    bool custom;

    INDIDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                        const std::string &binary, const std::string &family,
                        const std::string &skeleton = "", bool custom = false);
};

class INDIDriverCollection
{
public:
    /**
     * @brief 构造函数
     * @param path INDI驱动程序的路径，默认为"/usr/share/indi"
     */
    INDIDriverCollection(const std::string &path = "/usr/share/indi");

    /**
     * @brief 解析所有INDI驱动程序
     */
    void parseDrivers();

    /**
     * @brief 解析自定义的INDI驱动程序
     * @param drivers JSON格式的自定义驱动程序数据
     */
    void parseCustomDrivers(const json &drivers);

    /**
     * @brief 清除自定义的INDI驱动程序
     */
    void clearCustomDrivers();

    /**
     * @brief 根据标签获取INDI设备容器
     * @param label 设备的标签
     * @return 指向INDIDeviceContainer的shared_ptr
     */
    std::shared_ptr<INDIDeviceContainer> getByLabel(const std::string &label);

    /**
     * @brief 根据名称获取INDI设备容器
     * @param name 设备的名称
     * @return 指向INDIDeviceContainer的shared_ptr
     */
    std::shared_ptr<INDIDeviceContainer> getByName(const std::string &name);

    /**
     * @brief 根据二进制文件名获取INDI设备容器
     * @param binary 二进制文件名
     * @return 指向INDIDeviceContainer的shared_ptr
     */
    std::shared_ptr<INDIDeviceContainer> getByBinary(const std::string &binary);

    /**
     * @brief 获取所有INDI设备的家族关系
     * @return 包含家族关系的映射表，键为家族名称，值为设备名称的向量
     */
    std::map<std::string, std::vector<std::string>> getFamilies();

private:
    std::string path;                                          ///< INDI驱动程序的路径
    std::vector<std::string> files;                            ///< INDI驱动程序文件列表
    std::vector<std::shared_ptr<INDIDeviceContainer>> drivers; ///< INDI驱动程序容器列表
};

