#include "iproperty.hpp"
#include "uuid.hpp"

#include <random>

IProperty::IProperty()
{
    UUID::UUIDGenerator generator;
    message_uuid = generator.generateUUIDWithFormat();
}

std::string IProperty::toJson() const
{
    std::stringstream ss;
    ss << "{";
    ss << "\"device_name\":\"" << device_name << "\",";
    ss << "\"device_uuid\":\"" << device_uuid << "\",";
    ss << "\"message_uuid\":\"" << message_uuid << "\",";
    ss << "\"name\":\"" << name << "\",";
    ss << "\"value\":";

    if (value.type() == typeid(int))
    {
        ss << std::any_cast<int>(value);
    }
    else if (value.type() == typeid(double))
    {
        ss << std::any_cast<double>(value);
    }
    else if (value.type() == typeid(bool))
    {
        ss << std::boolalpha << std::any_cast<bool>(value);
    }
    else if (value.type() == typeid(std::string))
    {
        ss << "\"" << std::any_cast<std::string>(value) << "\"";
    }

    ss << "}";
    return ss.str();
}

std::string IProperty::toXml() const
{
    std::stringstream ss;
    ss << "<message>";
    ss << "<device_name>" << device_name << "</device_name>";
    ss << "<device_uuid>" << device_uuid << "</device_uuid>";
    ss << "<message_uuid>" << message_uuid << "</message_uuid>";
    ss << "<name>" << name << "</name>";
    ss << "<value>";

    if (value.type() == typeid(int))
    {
        ss << std::any_cast<int>(value);
    }
    else if (value.type() == typeid(double))
    {
        ss << std::any_cast<double>(value);
    }
    else if (value.type() == typeid(bool))
    {
        ss << std::boolalpha << std::any_cast<bool>(value);
    }
    else if (value.type() == typeid(std::string))
    {
        ss << std::any_cast<std::string>(value);
    }

    ss << "</value>";
    ss << "</message>";
    return ss.str();
}

const std::string &IProperty::GetMessageUUID() const
{
    return message_uuid;
}

void IProperty::SetMessageUUID(const std::string &uuid)
{
    message_uuid = uuid;
}

const std::string &IProperty::GetDeviceUUID() const
{
    return device_uuid;
}

void IProperty::SetDeviceUUID(const std::string &uuid)
{
    device_uuid = uuid;
}
