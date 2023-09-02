/*******************************************************************************
  Copyright(c) 2017 Jasem Mutlaq. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include "connectioninterface.h"

#include "defaultdevice.h"

namespace Connection
{
    const char *CONNECTION_TAB = "Connection";

    Interface::Interface(HYDROGEN::DefaultDevice *dev, Type type) : m_Device(dev), m_Type(type)
    {
        // Default handshake
        registerHandshake([]()
                          { return true; });
    }

    Interface::~Interface()
    {
    }

    const char *Interface::getDeviceName()
    {
        return m_Device->getDeviceName();
    }

    bool Interface::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
    {
        HYDROGEN_UNUSED(dev);
        HYDROGEN_UNUSED(name);
        HYDROGEN_UNUSED(states);
        HYDROGEN_UNUSED(names);
        HYDROGEN_UNUSED(n);
        return false;
    }

    bool Interface::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
    {
        HYDROGEN_UNUSED(dev);
        HYDROGEN_UNUSED(name);
        HYDROGEN_UNUSED(values);
        HYDROGEN_UNUSED(names);
        HYDROGEN_UNUSED(n);
        return false;
    }

    bool Interface::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
    {
        HYDROGEN_UNUSED(dev);
        HYDROGEN_UNUSED(name);
        HYDROGEN_UNUSED(texts);
        HYDROGEN_UNUSED(names);
        HYDROGEN_UNUSED(n);
        return false;
    }

    bool Interface::ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
                              char *formats[], char *names[], int n)
    {
        HYDROGEN_UNUSED(dev);
        HYDROGEN_UNUSED(name);
        HYDROGEN_UNUSED(sizes);
        HYDROGEN_UNUSED(blobsizes);
        HYDROGEN_UNUSED(blobs);
        HYDROGEN_UNUSED(formats);
        HYDROGEN_UNUSED(names);
        HYDROGEN_UNUSED(n);
        return false;
    }

    bool Interface::saveConfigItems(FILE *fp)
    {
        HYDROGEN_UNUSED(fp);
        return true;
    }

    void Interface::registerHandshake(std::function<bool()> callback)
    {
        Handshake = callback;
    }
}
