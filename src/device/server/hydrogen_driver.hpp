/*
 * hydrogen_driver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Hydrogen Web Driver

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.h"
#else
#include <unordered_map>
#endif

#include "atom/type/json.hpp"
using json = nlohmann::json;

class HydrogenDeviceContainer
{
public:
    std::string name;
    std::string label;
    std::string version;
    std::string binary;
    std::string family;
    std::string skeleton;
    bool custom;

    HydrogenDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                        const std::string &binary, const std::string &family,
                        const std::string &skeleton = "", bool custom = false);
};

class HydrogenDriverCollection
{
public:
    /**
     * @brief 构造函数
     */
    explicit HydrogenDriverCollection() = default;

    /**
     * @brief 解析所有Hydrogen驱动程序
     */
    bool parseDrivers(const std::string &path);

    /**
     * @brief 解析自定义的Hydrogen驱动程序
     * @param drivers JSON格式的自定义驱动程序数据
     */
    bool parseCustomDrivers(const json &drivers);

    /**
     * @brief 清除自定义的Hydrogen驱动程序
     */
    void clearCustomDrivers();

    /**
     * @brief 根据标签获取Hydrogen设备容器
     * @param label 设备的标签
     * @return 指向HydrogenDeviceContainer的shared_ptr
     */
    std::shared_ptr<HydrogenDeviceContainer> getByLabel(const std::string &label);

    /**
     * @brief 根据名称获取Hydrogen设备容器
     * @param name 设备的名称
     * @return 指向HydrogenDeviceContainer的shared_ptr
     */
    std::shared_ptr<HydrogenDeviceContainer> getByName(const std::string &name);

    /**
     * @brief 根据二进制文件名获取Hydrogen设备容器
     * @param binary 二进制文件名
     * @return 指向HydrogenDeviceContainer的shared_ptr
     */
    std::shared_ptr<HydrogenDeviceContainer> getByBinary(const std::string &binary);

    /**
     * @brief 获取所有Hydrogen设备的家族关系
     * @return 包含家族关系的映射表，键为家族名称，值为设备名称的向量
     */
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::vector<std::string>> getFamilies();
#else
    std::unordered_map<std::string, std::vector<std::string>> getFamilies();
#endif

private:
    std::string path;                                          ///< Hydrogen驱动程序的路径
    std::vector<std::string> files;                            ///< Hydrogen驱动程序文件列表
    std::vector<std::shared_ptr<HydrogenDeviceContainer>> drivers; ///< Hydrogen驱动程序容器列表
};

