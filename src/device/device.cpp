#include "device.hpp"
#include "property/uuid.hpp"

Device::Device(const std::string &name)
{
    device_logger.setCurrentModule(name);
    device_logger.enableAsyncLogging();
    _name = name;
    OpenAPT::UUID::UUIDGenerator generator;
    _uuid = generator.generateUUIDWithFormat();
}

auto Device::IAFindMessage(const std::string &identifier)
{
    return std::find_if(device_messages.begin(), device_messages.end(), [&](const MessageInfo &msg)
                        { return msg.message.GetMessageUUID() == identifier || msg.message.GetName() == identifier; });
}

void Device::IAInsertMessage(const OpenAPT::Property::IMessage &message,const std::string& task_name)
{
    MessageInfo info;
    info.message = message;
    info.task = getSimpleTask(task_name,{});

    device_messages.push_back(info);
    IANotifyObservers(message);
}

OpenAPT::Property::IMessage Device::IACreateMessage(const std::string &message_name, std::any message_value)
{
    OpenAPT::Property::IMessage message;
    message.name = message_name;
    message.device_name = _name;
    message.device_uuid = _uuid;
    message.value = message_value;
    return message;
}

void Device::IAUpdateMessage(const std::string &identifier, const OpenAPT::Property::IMessage &newMessage)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        OpenAPT::Property::IMessage oldMessage = it->message;
        it->message = newMessage;
        IANotifyObservers(newMessage, oldMessage);
    }
}

void Device::IARemoveMessage(const std::string &identifier)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        OpenAPT::Property::IMessage removedMessage = it->message;
        device_messages.erase(it);
        IANotifyObservers(removedMessage);
    }
}

OpenAPT::Property::IMessage *Device::IAGetMessage(const std::string &identifier)
{
    auto it = IAFindMessage(identifier);

    if (it != device_messages.end())
    {
        return &(it->message);
    }

    return nullptr;
}

void Device::IANotifyObservers(const OpenAPT::Property::IMessage &newMessage, const OpenAPT::Property::IMessage &oldMessage)
{
    for (const auto &observer : observers)
    {
        observer(newMessage, oldMessage);
    }
}

void Device::IANotifyObservers(const OpenAPT::Property::IMessage &removedMessage)
{
    for (const auto &observer : observers)
    {
        observer(removedMessage, removedMessage);
    }
}
