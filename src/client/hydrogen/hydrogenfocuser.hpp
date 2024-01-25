/*
 * hydrogenfocuser.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-10

Description: Hydrogen Focuser

**************************************************/

#pragma once

#include "hydrogendevice.hpp"
#include "core/focuser.hpp"

template <typename... Args>
class StringSwitch;

class HydrogenFocuser : public Focuser, public LithiumIndiClient
{

public:
    /**
     * @brief 构造函数，初始化 HydrogenFocuser 类
     *
     * @param name 焦距器名字
     */
    HydrogenFocuser(const std::string &name);

    /**
     * @brief 析构函数，释放 HydrogenFocuser 类相关资源
     *
     */
    ~HydrogenFocuser();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    /**
     * @brief 将电调移动到 position 位置
     *
     * @param position 相对移动的步数
     * @return bool 移动是否成功
     */
    virtual bool moveTo(const nlohmann::json &params);

    /**
     * @brief 将电调移动到绝对位置 position
     *
     * @param position 绝对位置步数
     * @return bool 移动是否成功
     */
    virtual bool moveToAbsolute(const nlohmann::json &params);

    /**
     * @brief 移动电调 step 个步长
     *
     * @param step 移动步数
     * @return bool 移动是否成功
     */
    virtual bool moveStep(const nlohmann::json &params);

    /**
     * @brief 移动电调至绝对步数位置
     *
     * @param step 绝对步数位置
     * @return bool 移动是否成功
     */
    virtual bool moveStepAbsolute(const nlohmann::json &params);

    /**
     * @brief 中止电调移动
     *
     * @return bool 操作是否成功
     */
    virtual bool AbortMove(const nlohmann::json &params);

    /**
     * @brief 获取电调最大位置
     *
     * @return int 电调最大位置
     */
    virtual int getMaxPosition(const nlohmann::json &params);

    /**
     * @brief 设置电调最大位置
     *
     * @param max_position 电调最大位置
     * @return bool 操作是否成功
     */
    virtual bool setMaxPosition(const nlohmann::json &params);

    /**
     * @brief 判断是否支持获取温度功能
     *
     * @return bool 是否支持获取温度功能
     */
    virtual bool isGetTemperatureAvailable(const nlohmann::json &params);

    /**
     * @brief 获取电调当前温度
     *
     * @return double 当前温度
     */
    virtual double getTemperature(const nlohmann::json &params);

    /**
     * @brief 判断是否支持绝对移动功能
     *
     * @return bool 是否支持绝对移动功能
     */
    virtual bool isAbsoluteMoveAvailable(const nlohmann::json &params);

    /**
     * @brief 判断是否支持手动移动功能
     *
     * @return bool 是否支持手动移动功能
     */
    virtual bool isManualMoveAvailable(const nlohmann::json &params);

    /**
     * @brief 获取电调当前位置
     *
     * @return int 当前位置
     */
    virtual int getCurrentPosition(const nlohmann::json &params);

    /**
     * @brief 判断电调是否存在反向间隙
     *
     * @return bool 是否存在反向间隙
     */
    virtual bool haveBacklash(const nlohmann::json &params);

    /**
     * @brief 设置电调反向间隙值
     *
     * @param value 反向间隙值
     * @return bool 操作是否成功
     */
    virtual bool setBacklash(const nlohmann::json &params);

protected:
    /**
     * @brief 清除状态
     *
     */
    void ClearStatus();

protected:
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
    std::shared_ptr<ISwitchVectorProperty> m_connection_prop;        // 连接属性指针
    std::shared_ptr<ISwitchVectorProperty> m_mode_prop;              // 焦距器模式（绝对或相对）属性指针
    std::shared_ptr<ISwitchVectorProperty> m_motion_prop;            // 焦距器运动方向（向内或向外）属性指针
    std::shared_ptr<INumberVectorProperty> m_speed_prop;             // 焦距器速度属性指针，默认为 1
    std::shared_ptr<INumberVectorProperty> m_absolute_position_prop; // 焦距器绝对位置属性指针
    std::shared_ptr<INumberVectorProperty> m_relative_position_prop; // 焦距器相对位置属性指针
    std::shared_ptr<INumberVectorProperty> m_max_position_prop;      // 焦距器最大位置属性指针
    std::shared_ptr<INumberVectorProperty> m_temperature_prop;       // 焦距器温度属性指针
    std::shared_ptr<ISwitchVectorProperty> m_rate_prop;              // 焦距器速率属性指针
    std::shared_ptr<INumberVectorProperty> m_delay_prop;             // 焦距器延迟属性指针
    std::shared_ptr<ISwitchVectorProperty> m_backlash_prop;          // 焦距器反向间隙属性指针
    std::shared_ptr<INumberVectorProperty> m_focuserinfo_prop;       // 焦距器用户信息属性指针
    INumber *m_hydrogen_max_position;                                    // 焦距器 hydrogen 最大位置属性指针
    INumber *m_hydrogen_focuser_temperature;                             // 焦距器 hydrogen 温度属性指针
    std::shared_ptr<ITextVectorProperty> focuser_port;                             // 焦距器端口属性指针
    HYDROGEN::BaseDevice *focuser_device;                          // 焦距器设备指针

    std::atomic_bool is_ready; // 是否就绪
    std::atomic_bool has_blob; // 是否有 BLOB 数据
    std::atomic_bool is_debug;
    std::atomic_bool is_connected;

    bool can_absolute_move = false;
    bool has_backlash = false;

    std::atomic_int current_mode;
    std::atomic_int m_current_absolute_position;
    std::atomic_int m_current_motion;
    std::atomic_int m_current_speed;
    std::atomic<double> m_current_temperature;

    int m_delay = 0;
    int m_max_position = 0;

    std::string hydrogen_focuser_port = ""; // 焦距器所选端口
    std::string hydrogen_focuser_rate = ""; // 焦距器所选速率

    std::string hydrogen_focuser_cmd;            // Hydrogen 命令字符串
    std::string hydrogen_focuser_exec = "";      // Hydrogen 设备执行文件路径
    std::string hydrogen_focuser_version = "";   // Hydrogen 设备固件版本
    std::string hydrogen_focuser_interface = ""; // Hydrogen 接口版本

    std::unique_ptr<StringSwitch<INumberVectorProperty *>> m_number_switch;
    std::unique_ptr<StringSwitch<ISwitchVectorProperty *>> m_switch_switch;
    std::unique_ptr<StringSwitch<ITextVectorProperty *>> m_text_switch;
};
