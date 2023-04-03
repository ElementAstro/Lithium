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

#ifndef _INDI_CLIENT_HPP_
#define _INDI_CLIENT_HPP_

#include <libindi/baseclient.h>
#include <libindi/basedevice.h>

class OpenAptIndiClient : public INDI::BaseClient
{
    bool m_disconnecting;

    public:
        OpenAptIndiClient();
        ~OpenAptIndiClient();

    public:
        bool connectServer()
    #if INDI_VERSION_MAJOR >= 2 || (INDI_VERSION_MINOR == 9 && INDI_VERSION_RELEASE == 9)
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

    #if INDI_VERSION_MAJOR >= 2
    protected: // old deprecated interface INDI Version < 2.0.0
        virtual void newDevice(INDI::BaseDevice *dp) = 0;
        virtual void removeDevice(INDI::BaseDevice *dp) = 0;
        virtual void newProperty(INDI::Property *property) = 0;
        virtual void removeProperty(INDI::Property *property) = 0;

        virtual void newMessage(INDI::BaseDevice *dp, int messageID) = 0;
        virtual void newBLOB(IBLOB *bp) = 0;
        virtual void newSwitch(ISwitchVectorProperty *svp) = 0;
        virtual void newNumber(INumberVectorProperty *nvp) = 0;
        virtual void newText(ITextVectorProperty *tvp) = 0;
        virtual void newLight(ILightVectorProperty *lvp) = 0;

    protected: // new interface INDI Version >= 2.0.0
        void newDevice(INDI::BaseDevice device) override
        {
            return newDevice((INDI::BaseDevice *)device);
        }

        void removeDevice(INDI::BaseDevice device) override
        {
            return removeDevice((INDI::BaseDevice *)device);
        }

        void newProperty(INDI::Property property) override
        {
            return newProperty((INDI::Property *)property);
        }

        void removeProperty(INDI::Property property) override
        {
            return removeProperty((INDI::Property *)property);
        }

        void updateProperty(INDI::Property property) override
        {
            switch (property.getType())
            {
            case INDI_NUMBER: return newNumber((INumberVectorProperty *)property);
            case INDI_SWITCH: return newSwitch((ISwitchVectorProperty *)property);
            case INDI_LIGHT:  return newLight((ILightVectorProperty *)property);
            case INDI_BLOB:   return newBLOB((IBLOB *)INDI::PropertyBlob(property)[0].cast());
            case INDI_TEXT:   return newText((ITextVectorProperty *)property);
            }
        }

        void newMessage(INDI::BaseDevice device, int messageID) override
        {
            return newMessage((INDI::BaseDevice *)device, messageID);
        }
    #endif
};

#endif