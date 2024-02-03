#include "docenum.hpp"

void DocIntEnum::addEnum(int value, std::string_view doc) {
    enumMap[value] = doc;
}

std::string_view DocIntEnum::getEnumDoc(int value) const {
    const auto it = enumMap.find(value);
    if (it != enumMap.end()) {
        return it->second;
    }
    return "No documentation available for the enum value.";
}

int DocIntEnum::getEnumValue(std::string_view doc) const {
    for (const auto& [key, value] : enumMap) {
        if (value == doc) {
            return key;
        }
    }
    return -1; // Return -1 if no enum value is found for the given documentation.
}

void DocIntEnum::removeEnum(int value) {
    enumMap.erase(value);
}

void DocIntEnum::clearEnums() {
    enumMap.clear();
}
