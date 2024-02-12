/*
 * hydrogenclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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

    // must use this in PHD2 rather than BaseClient::disconnectServer()
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