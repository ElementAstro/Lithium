/*
 * guider.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Guider Defination

*************************************************/

#pragma once

#include "device.hpp"

class Guider : public Device
{
public:
    /**
     * @brief 构造函数，创建一个名为 name 的电调对象
     *
     * @param name 电调名称
     */
    Guider(const std::string &name);

    /**
     * @brief 析构函数，释放资源
     */
    virtual ~Guider();

    virtual bool connect(const nlohmann::json &params) override;

    virtual bool disconnect(const nlohmann::json &params) override;

    virtual bool reconnect(const nlohmann::json &params) override;
};