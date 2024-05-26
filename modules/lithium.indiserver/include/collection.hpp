#ifndef LITHIUM_INDISERVER_COLLECTION_HPP
#define LITHIUM_INDISERVER_COLLECTION_HPP

#include "atom/type/json.hpp"
using json = nlohmann::json;

#include "container.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class INDIDriverCollection {
public:
    /**
     * @brief 解析所有INDI驱动程序
     */
    bool parseDrivers(const std::string &path);

    /**
     * @brief 解析自定义的INDI驱动程序
     * @param drivers JSON格式的自定义驱动程序数据
     */
    bool parseCustomDrivers(const json &drivers);

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
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::vector<std::string>> getFamilies();
#else
    std::unordered_map<std::string, std::vector<std::string>> getFamilies();
#endif

private:
    std::string path;                ///< INDI驱动程序的路径
    std::vector<std::string> files;  ///< INDI驱动程序文件列表
    std::vector<std::shared_ptr<INDIDeviceContainer>>
        drivers;  ///< INDI驱动程序容器列表
};

#endif
