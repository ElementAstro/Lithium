/*
 * indiclient.hpp
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

#ifndef _LITHIUM_CLIENT_HPP_
#define _LITHIUM_CLIENT_HPP_

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
    bool connectServer()
#if LITHIUM_VERSION_MAJOR >= 2 || (LITHIUM_VERSION_MINOR == 9 && LITHIUM_VERSION_RELEASE == 9)
        override // use override since 1.9.9
#endif
        ;

protected:
    void serverConnected() final;
    void serverDisconnected(int exit_code) final;

    virtual void IndiServerConnected() = 0;
    virtual void IndiServerDisconnected(int exit_code) = 0;

    // must use this in LGuider2 rather than BaseClient::disconnectServer()
    bool DisconnectIndiServer();

#if LITHIUM_VERSION_MAJOR >= 2
protected: // old deprecated interface INDI Version < 2.0.0
    virtual void newDevice(LITHIUM::BaseDevice *dp) = 0;
    virtual void removeDevice(LITHIUM::BaseDevice *dp) = 0;
    virtual void newProperty(LITHIUM::Property *property) = 0;
    virtual void removeProperty(LITHIUM::Property *property) = 0;

    virtual void newMessage(LITHIUM::BaseDevice *dp, int messageID) = 0;
    virtual void newBLOB(IBLOB *bp) = 0;
    virtual void newSwitch(ISwitchVectorProperty *svp) = 0;
    virtual void newNumber(INumberVectorProperty *nvp) = 0;
    virtual void newText(ITextVectorProperty *tvp) = 0;
    virtual void newLight(ILightVectorProperty *lvp) = 0;

protected: // new interface INDI Version >= 2.0.0
    void newDevice(LITHIUM::BaseDevice device) override
    {
        return newDevice((LITHIUM::BaseDevice *)device);
    }

    void removeDevice(LITHIUM::BaseDevice device) override
    {
        return removeDevice((LITHIUM::BaseDevice *)device);
    }

    void newProperty(LITHIUM::Property property) override
    {
        return newProperty((LITHIUM::Property *)property);
    }

    void removeProperty(LITHIUM::Property property) override
    {
        return removeProperty((LITHIUM::Property *)property);
    }

    void updateProperty(LITHIUM::Property property) override
    {
        switch (property.getType())
        {
        case LITHIUM_NUMBER:
            return newNumber((INumberVectorProperty *)property);
        case LITHIUM_SWITCH:
            return newSwitch((ISwitchVectorProperty *)property);
        case LITHIUM_LIGHT:
            return newLight((ILightVectorProperty *)property);
        case LITHIUM_BLOB:
            return newBLOB((IBLOB *)LITHIUM::PropertyBlob(property)[0].cast());
        case LITHIUM_TEXT:
            return newText((ITextVectorProperty *)property);
        }
    }

    void newMessage(LITHIUM::BaseDevice device, int messageID) override
    {
        return newMessage((LITHIUM::BaseDevice *)device, messageID);
    }
#endif
};

#endif