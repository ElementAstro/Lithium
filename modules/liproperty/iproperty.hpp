#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <any>
#include <vector>

class IProperty
{
public:
    IProperty();
    virtual ~IProperty() = default;

    const std::string &GetName() const
    {
        return name;
    }

    virtual std::string toJson() const;
    virtual std::string toXml() const;

    const std::string &GetMessageUUID() const;
    void SetMessageUUID(const std::string &uuid);

    const std::string &GetDeviceUUID() const;
    void SetDeviceUUID(const std::string &uuid);

    template <typename T>
    T getValue() const;

    template <typename T>
    void setValue(const T &value);

    std::string device_name;
    std::string device_uuid;

    std::string message_uuid;
    std::string name;

    std::any value;
};

template <typename T>
T IProperty::getValue() const
{
    try
    {
        return std::any_cast<T>(value);
    }
    catch (const std::bad_any_cast &)
    {
        throw std::runtime_error("Failed to get value from the message.");
    }
}

template <typename T>
void IProperty::setValue(const T &value)
{
    this->value = value;
}
