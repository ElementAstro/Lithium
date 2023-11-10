/*
 * hydrogenclient.hpp
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

Date: 2023-4-4

Description: INDI CLient Interface

**************************************************/

#ifndef _HYDROGEN_CLIENT_HPP_
#define _HYDROGEN_CLIENT_HPP_

#ifdef NATIVE_INDI
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#else
#include "hydrogen_client/baseclient.h"
#include "core/base/basedevice.h"
#endif

class LithiumIndiClient : public HYDROGEN::BaseClient
{
    bool m_disconnecting;

public:
    LithiumIndiClient();
    ~LithiumIndiClient();

public:
    bool connectServer() override;

protected:
    void serverConnected() final;
    void serverDisconnected(int exit_code) final;

    virtual void IndiServerConnected() = 0;
    virtual void IndiServerDisconnected(int exit_code) = 0;

    // must use this in LGuider2 rather than BaseClient::disconnectServer()
    bool DisconnectIndiServer();

protected: // old deprecated interface INDI Version < 2.0.0
    virtual void newDevice(HYDROGEN::BaseDevice *dp) = 0;
    virtual void removeDevice(HYDROGEN::BaseDevice *dp) = 0;
    virtual void newProperty(HYDROGEN::Property *property) = 0;
    virtual void removeProperty(HYDROGEN::Property *property) = 0;

    virtual void newMessage(HYDROGEN::BaseDevice *dp, int messageID) = 0;
    virtual void newBLOB(IBLOB *bp) = 0;
    virtual void newSwitch(ISwitchVectorProperty *svp) = 0;
    virtual void newNumber(INumberVectorProperty *nvp) = 0;
    virtual void newText(ITextVectorProperty *tvp) = 0;
    virtual void newLight(ILightVectorProperty *lvp) = 0;

protected: // new interface INDI Version >= 2.0.0
    void newDevice(HYDROGEN::BaseDevice device) override
    {
        return newDevice((HYDROGEN::BaseDevice *)device);
    }

    void removeDevice(HYDROGEN::BaseDevice device) override
    {
        return removeDevice((HYDROGEN::BaseDevice *)device);
    }

    void newProperty(HYDROGEN::Property property) override
    {
        return newProperty((HYDROGEN::Property *)property);
    }

    void removeProperty(HYDROGEN::Property property) override
    {
        return removeProperty((HYDROGEN::Property *)property);
    }

    void updateProperty(HYDROGEN::Property property) override
    {
        switch (property.getType())
        {
        case HYDROGEN_NUMBER:
            return newNumber((INumberVectorProperty *)property);
        case HYDROGEN_SWITCH:
            return newSwitch((ISwitchVectorProperty *)property);
        case HYDROGEN_LIGHT:
            return newLight((ILightVectorProperty *)property);
        case HYDROGEN_BLOB:
            return newBLOB((IBLOB *)HYDROGEN::PropertyBlob(property)[0].cast());
        case HYDROGEN_TEXT:
            return newText((ITextVectorProperty *)property);
        }
    }

    void newMessage(HYDROGEN::BaseDevice device, int messageID) override
    {
        return newMessage((HYDROGEN::BaseDevice *)device, messageID);
    }
};

#endif