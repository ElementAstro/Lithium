#pragma once

#include <curl/curl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

template <typename Type>
concept JsonCompatible = requires(json jsonObj, Type typeObj) {
    { jsonObj.get<Type>() } -> std::convertible_to<Type>;
};

class AnotherOperationException : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_ANOTHER_OPERATION(...)                                \
    throw AnotherOperationException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                    ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_ANOTHER_OPERATION(...)                                  \
    AnotherOperationException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                             ATOM_FUNC_NAME, __VA_ARGS__)

class AlpacaDevice {
public:
    AlpacaDevice(const std::string& address, const std::string& deviceType,
                 int deviceNumber, const std::string& protocol);
    virtual ~AlpacaDevice();

    // Common interface methods
    auto action(const std::string& actionName,
                const std::vector<std::string>& parameters = {}) -> std::string;
    auto commandBlind(const std::string& command, bool raw) -> void;
    auto commandBool(const std::string& command, bool raw) -> bool;
    auto commandString(const std::string& command, bool raw) -> std::string;

    auto getConnected() -> bool;
    auto setConnected(bool connectedState) -> void;
    auto getDescription() -> std::string;
    auto getDriverInfo() -> std::vector<std::string>;
    auto getDriverVersion() -> std::string;
    auto getInterfaceVersion() -> int;
    auto getName() -> std::string;
    auto getSupportedActions() -> std::vector<std::string>;

    template <Number Type>
    auto getNumericProperty(const std::string& propertyName) -> Type {
        return std::get<Type>(get(propertyName));
    }

    template <JsonCompatible Type>
    auto getArrayProperty(const std::string& property,
                          const std::map<std::string, std::string>& parameters =
                              {}) -> std::vector<Type> {
        json response = get(property, parameters);
        return response["Value"].get<std::vector<Type>>();
    }

protected:
    // HTTP methods
    auto get(const std::string& attribute,
             const std::map<std::string, std::string>& params = {}) -> json;
    auto put(const std::string& attribute,
             const std::map<std::string, std::string>& data = {}) -> json;

private:
    static auto writeCallback(void* contents, size_t size, size_t nmemb,
                              std::string* str) -> size_t;
    auto checkError(const json& response) -> void;

    std::string address_;
    std::string deviceType_;
    int deviceNumber_;
    int apiVersion_;
    std::string baseUrl_;

    static int clientId;
    static int clientTransId;
    static std::mutex clientTransIdMutex;

    std::unique_ptr<CURL, std::function<void(CURL*)>> curl_;
};
