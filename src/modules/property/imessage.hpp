#include <iostream>
#include <string>
#include <sstream>
#include <any>
#include <vector>

namespace Lithium::Property
{
    class IMessage
    {
    public:
        IMessage();
        virtual ~IMessage() = default;

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
    T IMessage::getValue() const
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
    void IMessage::setValue(const T &value)
    {
        this->value = value;
    }

    class IImage : public IMessage
    {
    public:
        int width;
        int height;
        int depth;

        int gain;
        int iso;
        int offset;
        int binning;

        double duration;

        bool is_color;

        std::string center_ra;
        std::string center_dec;

        std::string author;
        std::string time;
        std::string software = "Lithium-Server";

        std::string toJson() const override;
        std::string toXml() const override;
    };

    
}
