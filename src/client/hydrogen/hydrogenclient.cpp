/*
 * hydrogenclient.cpp
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

#include "hydrogenclient.hpp"

LithiumIndiClient::LithiumIndiClient()
    : m_disconnecting(false)
{
}

LithiumIndiClient::~LithiumIndiClient()
{
}

void LithiumIndiClient::serverDisconnected(int exit_code)
{
    m_disconnecting = true;
    IndiServerDisconnected(exit_code);
    m_disconnecting = false;
}

void LithiumIndiClient::serverConnected()
{
    // nothing to do yet
    // for INDI Core 1.9.9, 2.0.0, the function is called before requesting to retrieve device information.
    // If the function implementation waits for information, a deadlock occurs.
    //
    // see LithiumIndiClient::connectServer override function below
}

bool LithiumIndiClient::connectServer()
{
    // Call the original function.
    bool ok = HYDROGEN::BaseClient::connectServer();

    // After the original function was completed,
    // the device information request was made in the INDI Core library.

    // If connected, inform via the IndiServerConnected function,
    // which replaces the serverConnected function.
    if (ok)
    {
        IndiServerConnected();
    }

    return ok;
}

bool LithiumIndiClient::DisconnectIndiServer()
{
    // suppress any attempt to call disconnectServer from the
    // serverDisconnected callback.  Some serverDisconnected callbacks
    // in LGuider call disconnectServer which causes a crash (unhandled
    // exception) if the callbacks are invoked in the INDI listener
    // thread since disconnectServer will try to join the listener
    // thread, throwing a C++ runtime exception (deadlock)

    if (m_disconnecting)
    {
        return true;
    }

    return disconnectServer();
}