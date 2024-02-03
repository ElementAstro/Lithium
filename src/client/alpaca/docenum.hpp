#pragma once

#include <map>
#include <string>
#include <string_view>

class DocIntEnum {
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
