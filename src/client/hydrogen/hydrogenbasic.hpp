/*
 * hydrogenbasic.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Basic Template

**************************************************/

#pragma once

#include "hydrogendevice.hpp"

template <typename... Args>
class StringSwitch;

class HydrogenBasic :  public LithiumIndiClient
{

public:
    /**
     * @brief 构造函数
     *
     * @param name 望远镜名字
     */
    HydrogenBasic(const std::string &name);

    ~HydrogenBasic();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

protected:
    /**
     * @brief 清空状态
     *
     */
    void ClearStatus();

protected:
    // Hydrogen 回调函数
    void newDevice(HYDROGEN::BaseDevice *dp) override;
    void removeDevice(HYDROGEN::BaseDevice *dp) override;
    void newProperty(HYDROGEN::Property *property) override;
    void removeProperty(HYDROGEN::Property *property) override {}
    void newBLOB(IBLOB *bp) override;
    void newSwitch(ISwitchVectorProperty *svp) override;
    void newNumber(INumberVectorProperty *nvp) override;
    void newMessage(HYDROGEN::BaseDevice *dp, int messageID) override;
    void newText(ITextVectorProperty *tvp) override;
    void newLight(ILightVectorProperty *lvp) override {}
    void IndiServerConnected() override;
    void IndiServerDisconnected(int exit_code) override;

private:
    // Hydrogen 客户端参数
    std::shared_ptr<ISwitchVectorProperty> m_connection_prop;    // 连接属性指针
    std::shared_ptr<INumberVectorProperty> basicinfo_prop; // 望远镜信息属性指针
    std::shared_ptr<ITextVectorProperty> basic_port;       // 望远镜端口属性指针
    std::shared_ptr<ISwitchVectorProperty> rate_prop;          // 望远镜速率属性指针
    std::shared_ptr<ITextVectorProperty> basic_prop;
    HYDROGEN::BaseDevice *basic_device;                    // 望远镜设备指针

    std::atomic_bool is_ready; // 是否就绪
    std::atomic_bool has_blob; // 是否有 BLOB 数据
    std::atomic_bool is_debug;
    std::atomic_bool is_connected;

    std::string hydrogen_basic_port = ""; // 望远镜所选端口
    std::string hydrogen_basic_rate = ""; // 望远镜所选速率

    std::string hydrogen_basic_cmd;            // Hydrogen 命令字符串
    std::string hydrogen_basic_exec = "";      // Hydrogen 设备执行文件路径
    std::string hydrogen_basic_version = "";   // Hydrogen 设备固件版本
    std::string hydrogen_basic_interface = ""; // Hydrogen 接口版本

    std::unique_ptr<StringSwitch<INumberVectorProperty *>> m_number_switch;
    std::unique_ptr<StringSwitch<ISwitchVectorProperty *>> m_switch_switch;
    std::unique_ptr<StringSwitch<ITextVectorProperty *>> m_text_switch;
};