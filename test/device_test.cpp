#include <iostream>
#include <any>
#include <nlohmann/json.hpp>
#include <functional>
#include <memory>
#include <map>

namespace Lithium
{
    class SimpleTask
    {
    };

    namespace UUID
    {
        class UUIDGenerator
        {
        public:
            std::string generateUUIDWithFormat()
            {
                return "generated_UUID";
            }
        };
    }
}

class Device
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

    virtual void init();

    void setProperty(const std::string &name, const std::string &value);

    std::string getProperty(const std::string &name);

    void insertTask(const std::string &name, std::any defaultValue,
                    bool isBlock = false, std::shared_ptr<Lithium::SimpleTask> task = nullptr);

    virtual std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params) = 0;

    void insertMessage(const std::string &name, std::any value);

    void updateMessage(const std::string &name, const std::string &identifier, std::any newValue);

    void removeMessage(const std::string &name, const std::string &identifier);

    std::any getMessageValue(const std::string &name, const std::string &identifier);

    void addObserver(const std::function<void(std::any newValue, std::any oldValue)> &observer);

    void removeObserver(const std::function<void(std::any newValue, std::any oldValue)> &observer);

    void exportDeviceInfoToJson();

    Device &operator<<(const std::pair<std::string, std::string> &property);

    friend std::ostream &operator<<(std::ostream &os, const Device &device);

private:
    class DeviceInfo
    {
    public:
        std::map<std::string, std::string> properties;
        std::map<std::string, std::any> messages;
    };

    std::string _name;
    std::string _uuid;
    DeviceInfo device_info;
    std::vector<std::function<void(std::any, std::any)>> observers;
};

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
class MyDevice : public Device
{
public:
    explicit MyDevice(const std::string &name) : Device(name)
    {
        init();
    }
    ~MyDevice() override {}

    void init() override
    {
        Device::init();
    }

    std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params) override
    {
        return nullptr;
    }
};

int main()
{
    MyDevice device("MyDevice");

    std::function<void(std::any, std::any)> observer = [](std::any newValue, std::any oldValue)
    {
        if (oldValue.has_value())
        {
            if (oldValue.type() == typeid(int))
            {
                std::cout << "Old Value: " << std::any_cast<int>(oldValue) << std::endl;
            }
            else if (oldValue.type() == typeid(std::string))
            {
                std::cout << "Old Value: " << std::any_cast<std::string>(oldValue) << std::endl;
            }
        }

        if (newValue.has_value())
        {
            if (newValue.type() == typeid(int))
            {
                std::cout << "New Value: " << std::any_cast<int>(newValue) << std::endl;
            }
            else if (newValue.type() == typeid(std::string))
            {
                std::cout << "New Value: " << std::any_cast<std::string>(newValue) << std::endl;
            }
        }
    };

    device.addObserver(observer);

    // 初始化设备和属性设置...

    device.init();

    device << std::make_pair("attribute2", "value2");

    std::string attribute1 = device.getProperty("name");
    std::cout << "Attribute 1: " << attribute1 << std::endl;

    device.insertMessage("message1", 10);

    device.updateMessage("message1", "message1_identifier", 20);

    std::any messageValue = device.getMessageValue("message1", "message1_identifier");
    if (messageValue.has_value())
    {
        try
        {
            int value = std::any_cast<int>(messageValue);
            std::cout << "Message 1 Identifier Value: " << value << std::endl;
        }
        catch (const std::bad_any_cast &ex)
        {
            std::cout << "Failed to cast the value to int." << std::endl;
        }
    }

    device.removeMessage("message1", "message1_identifier");

    device.insertMessage("message2", 30);

    std::cout << device;

    device.exportDeviceInfoToJson();

    // 移除观察者
    device.removeObserver(observer);

    return 0;
}
