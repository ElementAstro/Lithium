#include "device.hpp"
#include "modules/property/uuid.hpp"

Device::Device(const std::string &name) : _name(name)
{
    Lithium::UUID::UUIDGenerator generator;
    _uuid = generator.generateUUIDWithFormat();
}

Device::~Device() {}

void Device::init()
{
    setProperty("name", _name);
}

void Device::setProperty(const std::string &name, const std::string &value)
{
    std::string oldValue = getProperty(name);

    device_info.properties[name] = value;

    if (device_info.messages.find(name) == device_info.messages.end())
    {
        insertMessage(name, value);
    }
    else
    {
        updateMessage(name, name, value);
    }
}

std::string Device::getProperty(const std::string &name)
{
    if (device_info.properties.find(name) != device_info.properties.end())
    {
        return device_info.properties[name];
    }
    return "";
}

void Device::insertTask(const std::string &name, std::any defaultValue,
                        bool isBlock, std::shared_ptr<Lithium::SimpleTask> task)
{
    // TODO: Implement task insertion logic
}

void Device::insertMessage(const std::string &name, std::any value)
{
    Lithium::IMessage message;
    message.name = name;
    message.value = value;
    device_info.messages[name] = message;

    for (const auto &observer : observers)
    {
        observer(message);
    }
}

void Device::updateMessage(const std::string &name, const std::string &identifier, std::any newValue)
{
    if (device_info.messages.find(identifier) != device_info.messages.end())
    {
        Lithium::IMessage message;
        message.value = newValue;
        message.name = name;
        device_info.messages[identifier] = message;

        for (const auto &observer : observers)
        {
            observer(message);
        }
    }
}

void Device::removeMessage(const std::string &name, const std::string &identifier)
{
    if (device_info.messages.find(identifier) != device_info.messages.end())
    {
        Lithium::IMessage message = device_info.messages[identifier];
        device_info.messages.erase(identifier);

        for (const auto &observer : observers)
        {
            observer(message);
        }
    }
}

std::any Device::getMessageValue(const std::string &name, const std::string &identifier)
{
    if (device_info.messages.find(identifier) != device_info.messages.end())
    {
        return device_info.messages[identifier];
    }

    return nullptr;
}

void Device::addObserver(const std::function<void(const Lithium::IMessage &message)> &observer)
{
    observers.push_back(observer);
}

void Device::removeObserver(const std::function<void(const Lithium::IMessage &message)> &observer)
{
    observers.erase(std::remove_if(observers.begin(), observers.end(),
                                   [&observer](const std::function<void(const Lithium::IMessage &message)> &o)
                                   {
                                       return o.target<std::function<void(const Lithium::IMessage &message)>>() == observer.target<std::function<void(const Lithium::IMessage &message)>>();
                                   }),
                    observers.end());
}

void Device::exportDeviceInfoToJson()
{
    nlohmann::json jsonInfo;

    for (const auto &kv : device_info.properties)
    {
        jsonInfo[kv.first] = kv.second;
    }

    std::string jsonStr = jsonInfo.dump();

    std::cout << jsonStr << std::endl;
}

Device &Device::operator<<(const std::pair<std::string, std::string> &property)
{
    setProperty(property.first, property.second);
    return *this;
}

std::ostream &operator<<(std::ostream &os, const Device &device)
{
    nlohmann::json jsonInfo;
    jsonInfo["Device Name"] = device._name;
    jsonInfo["Device UUID"] = device._uuid;

    nlohmann::json propertiesJson;
    for (const auto &kv : device.device_info.properties)
    {
        propertiesJson[kv.first] = kv.second;
    }

    jsonInfo["Device Properties"] = propertiesJson;

    os << jsonInfo.dump(4); // 输出缩进格式的JSON字符串，缩进为4个空格

    return os;
}
