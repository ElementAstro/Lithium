#include "lithiumbase.h"
#include "basedevice.h"

namespace LITHIUM
{

    // device

    void BaseMediator::newDevice(LITHIUM::BaseDevice)
    {
    }

    void BaseMediator::removeDevice(LITHIUM::BaseDevice)
    {
    }

    // property

    void BaseMediator::newProperty(LITHIUM::Property)
    {
    }

    void BaseMediator::updateProperty(LITHIUM::Property)
    {
    }

    void BaseMediator::removeProperty(LITHIUM::Property)
    {
    }

    // message

    void BaseMediator::newMessage(LITHIUM::BaseDevice, int)
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
