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

    if (value != oldValue)
    {
        insertMessage(name, value);
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
    device_info.messages[name] = value;

    for (const auto &observer : observers)
    {
        observer(value, nullptr);
    }
}

void Device::updateMessage(const std::string &name, const std::string &identifier, std::any newValue)
{
    if (device_info.messages.find(identifier) != device_info.messages.end())
    {
        std::any oldValue = device_info.messages[identifier];
        device_info.messages[identifier] = newValue;

        for (const auto &observer : observers)
        {
            observer(newValue, oldValue);
        }
    }
}

void Device::removeMessage(const std::string &name, const std::string &identifier)
{
    if (device_info.messages.find(identifier) != device_info.messages.end())
    {
        std::any value = device_info.messages[identifier];
        device_info.messages.erase(identifier);

        for (const auto &observer : observers)
        {
            observer(nullptr, value);
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

void Device::addObserver(const std::function<void(std::any newValue, std::any oldValue)> &observer)
{
    observers.push_back(observer);
}

void Device::removeObserver(const std::function<void(std::any newValue, std::any oldValue)> &observer)
{
    observers.erase(std::remove_if(observers.begin(), observers.end(),
                                   [&observer](const std::function<void(std::any, std::any)> &o)
                                   {
                                       return o.target<std::function<void(std::any, std::any)>>() == observer.target<std::function<void(std::any, std::any)>>();
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
