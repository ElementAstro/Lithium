#include "hydrogenbase.h"
#include "basedevice.h"

namespace HYDROGEN
{

    // device

    void BaseMediator::newDevice(HYDROGEN::BaseDevice)
    {
    }

    void BaseMediator::removeDevice(HYDROGEN::BaseDevice)
    {
    }

    // property

    void BaseMediator::newProperty(HYDROGEN::Property)
    {
    }

    void BaseMediator::updateProperty(HYDROGEN::Property)
    {
    }

    void BaseMediator::removeProperty(HYDROGEN::Property)
    {
    }

    // message

    void BaseMediator::newMessage(HYDROGEN::BaseDevice, int)
    {
    }

    // server

    void BaseMediator::serverConnected()
    {
    }

    void BaseMediator::serverDisconnected(int)
    {
    }

}
