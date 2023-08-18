#include "device.hpp"
#include "liproperty/uuid.hpp"

#include <format>

Device::Device(const std::string &name) : _name(name)
{
    UUID::UUIDGenerator generator;
    _uuid = generator.generateUUIDWithFormat();
}

Device::~Device() {}

void Device::init()
{
    setStringProperty("name", _name);
    setStringProperty("uuid", _uuid);
}

void Device::insertStringProperty(const std::string &name, const std::string &value,
                                  std::vector<std::string> possible_values,
                                  PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<IStringProperty> property = std::make_shared<IStringProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    string_properties[name] = property;
    for (const auto &observer : string_observers)
    {
        observer(property);
    }
}

void Device::insertNumberBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const double &value, std::vector<double> possible_values, PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<INumberProperty> property = std::make_shared<INumberProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    property->get_func = bind_get_func;
    property->set_func = bind_set_func;
    number_properties[name] = property;
    for (const auto &observer : number_observers)
    {
        observer(property);
    }
}

void Device::insertNumberProperty(const std::string &name, const double &value,
                                  std::vector<double> possible_values,
                                  PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<INumberProperty> property = std::make_shared<INumberProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    number_properties[name] = property;
    for (const auto &observer : number_observers)
    {
        observer(property);
    }
}

void Device::insertStringBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const std::string &value, std::vector<std::string> possible_values, PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<IStringProperty> property = std::make_shared<IStringProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    property->get_func = bind_get_func;
    property->set_func = bind_set_func;
    string_properties[name] = property;
    for (const auto &observer : string_observers)
    {
        observer(property);
    }
}

void Device::insertBoolProperty(const std::string &name, const bool &value,
                                std::vector<bool> possible_values,
                                PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<IBoolProperty> property = std::make_shared<IBoolProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    bool_properties[name] = property;
    for (const auto &observer : bool_observers)
    {
        observer(property);
    }
}

void Device::insertBoolBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const bool &value, std::vector<bool> possible_values, PossibleValueType possible_type, bool need_check)
{
    std::shared_ptr<IBoolProperty> property = std::make_shared<IBoolProperty>();
    property->device_name = _name;
    property->name = name;
    property->device_uuid = _uuid;
    property->value = value;
    property->need_check = need_check;
    property->pv_type = possible_type;
    property->possible_values = possible_values;
    bool_properties[name] = property;
    property->get_func = bind_get_func;
    property->set_func = bind_set_func;
    for (const auto &observer : bool_observers)
    {
        observer(property);
    }
}

std::shared_ptr<IStringProperty> Device::getStringProperty(const std::string &name)
{
    if (string_properties.find(name) != string_properties.end())
    {
        if (!string_properties[name]->get_func.empty())
        {
            invokeCommand(std::format("get_{}", name));
        }
        return string_properties[name];
    }
    return nullptr;
}

std::shared_ptr<INumberProperty> Device::getNumberProperty(const std::string &name)
{
    if (number_properties.find(name) != number_properties.end())
    {
        if (!number_properties[name]->get_func.empty())
        {
            invokeCommand(std::format("get_{}", name));
        }
        return number_properties[name];
    }
    return nullptr;
}

std::shared_ptr<IBoolProperty> Device::getBoolProperty(const std::string &name)
{
    if (bool_properties.find(name) != bool_properties.end())
    {
        if (!bool_properties[name]->get_func.empty())
        {
            invokeCommand(std::format("get_{}", name));
        }
        return bool_properties[name];
    }
    return nullptr;
}

void Device::setNumberProperty(const std::string &name, const double &value)
{
    if (number_properties.find(name) != number_properties.end())
    {
        number_properties[name]->value = value;
        if (!number_properties[name]->set_func.empty())
        {
            invokeCommand(std::format("set_{}", name));
        }
    }
}

void Device::setStringProperty(const std::string &name, const std::string &value)
{
    if (string_properties.find(name) != string_properties.end())
    {
        string_properties[name]->value = value;
        if (!string_properties[name]->set_func.empty())
        {
            invokeCommand(std::format("set_{}", name));
        }
    }
}

void Device::setBoolProperty(const std::string &name, const bool &value)
{
    if (bool_properties.find(name) != bool_properties.end())
    {
        bool_properties[name]->value = value;
        if (!bool_properties[name]->set_func.empty())
        {
            invokeCommand(std::format("set_{}", name));
        }
    }
}

void Device::removeStringProperty(const std::string &name)
{
    if (string_properties.find(name) != string_properties.end())
    {
        string_properties.erase(name);
    }
}

void Device::removeNumberProperty(const std::string &name)
{
    if (number_properties.find(name) != number_properties.end())
    {
        number_properties.erase(name);
    }
}

void Device::removeBoolProperty(const std::string &name)
{
    if (bool_properties.find(name) != bool_properties.end())
    {
        bool_properties.erase(name);
    }
}

void Device::insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                        const std::function<nlohmann::json(const nlohmann::json &)> &func,
                        const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                        bool isBlock)
{
    if (name.empty() || !defaultValue.has_value())
    {
        return;
    }
    if (!stop_func)
    {
        task_map[name] = std::make_shared<DeviceTask>(func, params_template, _name, _uuid, _name, stop_func, false);
    }
    else
    {
        task_map[name] = std::make_shared<DeviceTask>(func, params_template, _name, _uuid, _name, stop_func, true);
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

void Device::addStringObserver(const std::function<void(const std::shared_ptr<IStringProperty> &message)> &observer)
{
    string_observers.push_back(observer);
}

void Device::addNumberObserver(const std::function<void(const std::shared_ptr<INumberProperty> &message)> &observer)
{
    number_observers.push_back(observer);
}

void Device::addBoolObserver(const std::function<void(const std::shared_ptr<IBoolProperty> &message)> &observer)
{
    bool_observers.push_back(observer);
}

void Device::removeStringObserver(const std::function<void(const std::shared_ptr<IStringProperty> &message)> &observer)
{
    string_observers.erase(std::remove_if(string_observers.begin(), string_observers.end(),
                                          [&observer](const std::function<void(const std::shared_ptr<IStringProperty> &message)> &o)
                                          {
                                              return o.target<std::function<void(const std::shared_ptr<IStringProperty> &message)>>() == observer.target<std::function<void(const std::shared_ptr<IStringProperty> &message)>>();
                                          }),
                           string_observers.end());
}

void Device::removeNumberObserver(const std::function<void(const std::shared_ptr<INumberProperty> &message)> &observer)
{
    number_observers.erase(std::remove_if(number_observers.begin(), number_observers.end(),
                                          [&observer](const std::function<void(const std::shared_ptr<INumberProperty> &message)> &o)
                                          {
                                              return o.target<std::function<void(const std::shared_ptr<INumberProperty> &message)>>() == observer.target<std::function<void(const std::shared_ptr<INumberProperty> &message)>>();
                                          }),
                           number_observers.end());
}

void Device::removeBoolObserver(const std::function<void(const std::shared_ptr<IBoolProperty> &message)> &observer)
{
    bool_observers.erase(std::remove_if(bool_observers.begin(), bool_observers.end(),
                                        [&observer](const std::function<void(const std::shared_ptr<IBoolProperty> &message)> &o)
                                        {
                                            return o.target<std::function<void(const std::shared_ptr<IBoolProperty> &message)>>() == observer.target<std::function<void(const std::shared_ptr<IBoolProperty> &message)>>();
                                        }),
                         bool_observers.end());
}

const nlohmann::json Device::exportDeviceInfoToJson()
{
    nlohmann::json jsonInfo;
    for (const auto &kv : string_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    for (const auto &kv : number_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    for (const auto &kv : bool_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    std::cout << jsonInfo.dump(4) << std::endl;
    return jsonInfo;
}
