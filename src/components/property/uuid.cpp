#include "uuid.hpp"

namespace Lithium::UUID
{
    UUIDGenerator::UUIDGenerator() : gen_(std::random_device{}()), dis_(0, 15)
    {
    }

    void UUIDGenerator::seed(unsigned int seed_value)
    {
        gen_.seed(seed_value);
    }

    unsigned int UUIDGenerator::getRandomNumber()
    {
        return dis_(gen_);
    }

    std::string UUIDGenerator::generateUUID()
    {
        std::stringstream ss;
        for (int i = 0; i < 32; ++i)
        {
            if (i == 8 || i == 12 || i == 16 || i == 20)
                ss << '-';
            ss << std::hex << getRandomNumber();
        }
        return ss.str();
    }

    std::string UUIDGenerator::generateUUIDWithFormat(bool use_braces, bool use_hyphens)
    {
        std::stringstream ss;
        if (use_braces)
            ss << '{';
        ss << std::hex << getRandomNumber();
        if (use_hyphens)
            ss << '-';
        ss << std::hex << getRandomNumber();
        ss << '-';
        ss << std::hex << getRandomNumber();
        if (use_hyphens)
            ss << '-';
        ss << std::hex << getRandomNumber();
        ss << '-';
        for (int i = 0; i < 12; ++i)
            ss << std::hex << getRandomNumber();
        if (use_braces)
            ss << '}';
        return ss.str();
    }

}

/*
#include <iostream>

int main()
{
    Lithium::UUID::UUIDGenerator generator;

    // 设置随机数种子
    generator.seed(123);

    // 生成一个 UUID
    std::string uuid = generator.generateUUID();
    std::cout << "Generated UUID: " << uuid << std::endl;

    // 使用指定格式生成一个 UUID
    std::string formattedUUID = generator.generateUUIDWithFormat();
    std::cout << "Formatted UUID: " << formattedUUID << std::endl;

    return 0;
}
*/
