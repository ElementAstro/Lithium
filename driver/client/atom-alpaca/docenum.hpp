/*
 * docenum.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: Enum with documents just like Python

**************************************************/

#ifndef ATOM_ALPACA_DOCENUM_HPP
#define ATOM_ALPACA_DOCENUM_HPP

#include <map>
#include <string>
#include <string_view>

class DocIntEnum
{
private:
    std::map<int, std::string_view> enumMap;

public:
    DocIntEnum() = default;

    void addEnum(int value, std::string_view doc);

    [[nodiscard]] std::string_view getEnumDoc(int value) const;

    [[nodiscard]] int getEnumValue(std::string_view doc) const;

    void removeEnum(int value);

    void clearEnums();
};

#endif
