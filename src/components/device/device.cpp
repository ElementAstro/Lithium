#include "device.hpp"
#include "property/uuid.hpp"

Device::Device(const std::string &name)
{
    _name = name;
    Lithium::UUID::UUIDGenerator generator;
    _uuid = generator.generateUUIDWithFormat();
}

auto Device::IAFindMessage(const std::string &identifier)
{
    return std::find_if(device_messages.begin(), device_messages.end(), [&](const MessageInfo &msg)
                        { return msg.message.GetMessageUUID() == identifier || msg.message.GetName() == identifier; });
}

void Device::IAInsertMessage(const Lithium::Property::IMessage &message, std::shared_ptr<Lithium::SimpleTask> task)
{
    MessageInfo info;
    info.message = message;
    info.task = task;

    device_messages.push_back(info);
    IANotifyObservers(message);
}

Lithium::Property::IMessage Device::IACreateMessage(const std::string &message_name, std::any message_value)
{
    Lithium::Property::IMessage message;
    message.name = message_name;
    message.device_name = _name;
    message.device_uuid = _uuid;
    message.value = message_value;
    return message;
}

void Device::IAUpdateMessage(const std::string &identifier, const Lithium::Property::IMessage &newMessage)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        Lithium::Property::IMessage oldMessage = it->message;
        it->message = newMessage;
        IANotifyObservers(newMessage, oldMessage);
    }
}

void Device::IARemoveMessage(const std::string &identifier)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        Lithium::Property::IMessage removedMessage = it->message;
        device_messages.erase(it);
        IANotifyObservers(removedMessage);
    }
}

Lithium::Property::IMessage *Device::IAGetMessage(const std::string &identifier)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        return &(it->message);
    }

    return nullptr;
}

void Device::IANotifyObservers(const Lithium::Property::IMessage &newMessage, const Lithium::Property::IMessage &oldMessage)
{
    for (const auto &observer : observers)
    {
        observer(newMessage, oldMessage);
    }
}

void Device::IANotifyObservers(const Lithium::Property::IMessage &removedMessage)
{
    for (const auto &observer : observers)
    {
        observer(removedMessage, removedMessage);
    }
}
