#include "imessage.hpp"
#include "uuid.hpp"

#include <random>

namespace OpenAPT::Property
{
    IMessage::IMessage()
    {
        UUID::UUIDGenerator generator;
        message_uuid = generator.generateUUIDWithFormat();
    }

    std::string IMessage::toJson() const
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

    std::string IMessage::toXml() const
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

    const std::string &IMessage::GetMessageUUID() const
    {
        return message_uuid;
    }

    void IMessage::SetMessageUUID(const std::string &uuid)
    {
        message_uuid = uuid;
    }

    const std::string &IMessage::GetDeviceUUID() const
    {
        return device_uuid;
    }

    void IMessage::SetDeviceUUID(const std::string &uuid)
    {
        device_uuid = uuid;
    }

    std::string IImage::toJson() const
    {
        std::stringstream ss;
        ss << "{";
        ss << "\"device_name\":\"" << device_name << "\",";
        ss << "\"device_uuid\":\"" << device_uuid << "\",";
        ss << "\"message_uuid\":\"" << message_uuid << "\",";
        ss << "\"name\":\"" << name << "\",";
        ss << "\"value\":{";
        ss << "\"width\":" << width << ",";
        ss << "\"height\":" << height << ",";
        ss << "\"depth\":" << depth << ",";
        ss << "\"gain\":" << gain << ",";
        ss << "\"iso\":" << iso << ",";
        ss << "\"offset\":" << offset << ",";
        ss << "\"binning\":" << binning << ",";
        ss << "\"duration\":" << duration << ",";
        ss << "\"is_color\":" << std::boolalpha << is_color << ",";
        ss << "\"center_ra\":\"" << center_ra << "\",";
        ss << "\"center_dec\":\"" << center_dec << "\",";
        ss << "\"author\":\"" << author << "\",";
        ss << "\"time\":\"" << time << "\",";
        ss << "\"software\":\"" << software << "\"";
        ss << "}";
        ss << "}";
        return ss.str();
    }

    std::string IImage::toXml() const
    {
        std::stringstream ss;
        ss << "<message>";
        ss << "<device_name>" << device_name << "</device_name>";
        ss << "<device_uuid>" << device_uuid << "</device_uuid>";
        ss << "<message_uuid>" << message_uuid << "</message_uuid>";
        ss << "<name>" << name << "</name>";
        ss << "<value>";
        ss << "<width>" << width << "</width>";
        ss << "<height>" << height << "</height>";
        ss << "<depth>" << depth << "</depth>";
        ss << "<gain>" << gain << "</gain>";
        ss << "<iso>" << iso << "</iso>";
        ss << "<offset>" << offset << "</offset>";
        ss << "<binning>" << binning << "</binning>";
        ss << "<duration>" << duration << "</duration>";
        ss << "<is_color>" << std::boolalpha << is_color << "</is_color>";
        ss << "<center_ra>" << center_ra << "</center_ra>";
        ss << "<center_dec>" << center_dec << "</center_dec>";
        ss << "<author>" << author << "</author>";
        ss << "<time>" << time << "</time>";
        ss << "<software>" << software << "</software>";
        ss << "</value>";
        ss << "</message>";
        return ss.str();
    }
} // namespace OpenAPT::Property
