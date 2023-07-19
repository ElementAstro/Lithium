#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

bool validateJsonValue(const json &data, const json &templateValue)
{
    // Check if the value type is correct
    if (data.type() != templateValue.type())
    {
        // Allow different value types as long as templateValue is not empty
        if (templateValue.empty())
        {
            return false;
        }
    }

    // If the value is an object, recursively validate its elements
    if (data.is_object())
    {
        for (auto it = templateValue.begin(); it != templateValue.end(); ++it)
        {
            const std::string &key = it.key();
            const auto &subTemplateValue = it.value();
            if (!validateJsonValue(data.value(key, json()), subTemplateValue))
            {
                return false;
            }
        }
    }

    // If the value is an array, recursively validate its elements
    else if (data.is_array())
    {
        if (templateValue.size() > 0 && data.size() != templateValue.size())
        {
            return false;
        }

        for (size_t i = 0; i < data.size(); ++i)
        {
            if (!validateJsonValue(data[i], templateValue[0]))
            {
                return false;
            }
        }
    }

    return true;
}

bool validateJsonString(const std::string &jsonString, const std::string &templateString)
{
    // Parse the JSON strings
    json jsonData;
    json templateData;

    try
    {
        jsonData = json::parse(jsonString);
        templateData = json::parse(templateString);
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON: " << e.what() << std::endl;
        return false;
    }

    // Validate the JSON data
    bool isValid = validateJsonValue(jsonData, templateData);

    if (isValid)
    {
        std::cout << "JSON validation passed!" << std::endl;
    }
    else
    {
        std::cout << "JSON validation failed!" << std::endl;
    }

    return isValid;
}

int main()
{
    std::string jsonString = R"({
        "name": "xxx",
        "age": 25
    })";

    std::string templateString = R"({
        "name": {
            "needed": true,
            "default": "xxx"
        },
        "age": {
            "needed": true,
            "default": -1
        }
    })";

    // Validate the JSON string
    validateJsonString(jsonString, templateString);

    return 0;
}
