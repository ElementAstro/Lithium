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
    setProperty("uuid", _uuid);
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

void Device::insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                        const std::function<nlohmann::json(const nlohmann::json &)> &func,
                        const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                        bool isBlock, std::shared_ptr<Lithium::SimpleTask> task)
{
    if (name.empty() || !defaultValue.has_value() || !task)
    {
        return;
    }
    if(!stop_func)
    {
        task_map[name] = std::make_shared<DeviceTask>(func,params_template,getProperty("name"),getProperty("uuid"),getProperty("name"),stop_func,false);
    }
    else
    {
        task_map[name] = std::make_shared<DeviceTask>(func,params_template,getProperty("name"),getProperty("uuid"),getProperty("name"),stop_func,true);
    }
}

bool Device::removeTask(const std::string &name)
{
    if (name.empty())
    {
        return false;
    }
    if (task_map.find(name) != task_map.end())
    {
        task_map.erase(name);
    }
    return true;
}

std::shared_ptr<Lithium::SimpleTask> Device::getTask(const std::string &name, const nlohmann::json &params)
{
    if (name.empty())
    {
        return nullptr;
    }
    if (task_map.find(name) != task_map.end())
    {
        auto tmp_task = task_map[name];
        tmp_task->SetParams(params);
        if (tmp_task->validateJsonValue(params, tmp_task->GetParamsTemplate()))
        {
            return tmp_task;
        }
    }
    return nullptr;
}

void Device::insertMessage(const std::string &name, std::any value)
{
    Lithium::IProperty message;
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
        Lithium::IProperty message;
        message.value = newValue;
        message.name = name;
        device_info.messages[identifier] = std::move(message);

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
        Lithium::IProperty message = device_info.messages[identifier];
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

void Device::addObserver(const std::function<void(const Lithium::IProperty &message)> &observer)
{
    observers.push_back(observer);
}

void Device::removeObserver(const std::function<void(const Lithium::IProperty &message)> &observer)
{
    observers.erase(std::remove_if(observers.begin(), observers.end(),
                                   [&observer](const std::function<void(const Lithium::IProperty &message)> &o)
                                   {
                                       return o.target<std::function<void(const Lithium::IProperty &message)>>() == observer.target<std::function<void(const Lithium::IProperty &message)>>();
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

    os << jsonInfo.dump(4);

    return os;
}
