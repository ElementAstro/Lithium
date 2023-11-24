/*
 * hydrogenfilterwheel.hpp
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

Date: 2023-4-10

Description: Hydrogen Filterwheel

**************************************************/

#pragma once

#include "hydrogendevice.hpp"
#include "core/filterwheel.hpp"

template <typename... Args>
class StringSwitch;

class HydrogenFilterwheel : public Filterwheel, public LithiumIndiClient
{
public:
    HydrogenFilterwheel(const std::string &name);
    ~HydrogenFilterwheel();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    virtual bool moveTo(const json &params) override;

    virtual bool getCurrentPosition(const json &params) override;

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

protected:

    void ClearStatus();

    // Hydrogen Parameters
private:
    std::shared_ptr<ISwitchVectorProperty> m_connection_prop;
    std::shared_ptr<INumberVectorProperty> filterinfo_prop;
    std::shared_ptr<ITextVectorProperty> filter_port;
    std::shared_ptr<ISwitchVectorProperty> rate_prop;
    std::shared_ptr<ITextVectorProperty> filter_prop;
    HYDROGEN::BaseDevice *filter_device;

    std::atomic_bool is_ready; // 是否就绪
    std::atomic_bool has_blob; // 是否有 BLOB 数据
    std::atomic_bool is_debug;
    std::atomic_bool is_connected;

    std::string hydrogen_filter_port = "";
    std::string hydrogen_filter_rate = "";

    std::string hydrogen_filter_cmd;
    std::string hydrogen_filter_exec = "";
    std::string hydrogen_filter_version = "";
    std::string hydrogen_filter_interface = "";

    std::unique_ptr<StringSwitch<INumberVectorProperty *>> m_number_switch;
    std::unique_ptr<StringSwitch<ISwitchVectorProperty *>> m_switch_switch;
    std::unique_ptr<StringSwitch<ITextVectorProperty *>> m_text_switch;
};
