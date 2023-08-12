#pragma once

#include <stdexcept>
#include <string>

class InvalidDeviceType : public std::exception
{
public:
    InvalidDeviceType(const std::string &message) : m_message(message) {}

    const char *what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};
