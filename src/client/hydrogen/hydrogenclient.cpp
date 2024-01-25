/*
 * hydrogenclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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