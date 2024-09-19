#include "device.hpp"
#include <format>
#include <random>
#include <sstream>
#include <stdexcept>

#include "atom/error/exception.hpp"

int AlpacaDevice::clientId = std::random_device{}() % 65536;
int AlpacaDevice::clientTransId = 1;
std::mutex AlpacaDevice::clientTransIdMutex;

AlpacaDevice::AlpacaDevice(const std::string& address,
                           const std::string& deviceType, int deviceNumber,
                           const std::string& protocol)
    : address_(address),
      deviceType_(deviceType),
      deviceNumber_(deviceNumber),
      apiVersion_(1) {
    baseUrl_ = std::format("{}://{}/api/v{}/{}/{}", protocol, address,
                           apiVersion_, deviceType, deviceNumber);

    curl_ = std::unique_ptr<CURL, std::function<void(CURL*)>>(
        curl_easy_init(), [](CURL* curl) { curl_easy_cleanup(curl); });
    if (!curl_) {
        THROW_CURL_INITIALIZATION_ERROR("Failed to initialize CURL");
    }
}

AlpacaDevice::~AlpacaDevice() = default;

auto AlpacaDevice::action(const std::string& actionName,
                          const std::vector<std::string>& parameters)
    -> std::string {
    json params = {{"Action", actionName}, {"Parameters", parameters}};
    return put("action", params)["Value"];
}

void AlpacaDevice::commandBlind(const std::string& command, bool raw) {
    put("commandblind",
        {{"Command", command}, {"Raw", raw ? "true" : "false"}});
}

auto AlpacaDevice::commandBool(const std::string& command, bool raw) -> bool {
    return put("commandbool", {{"Command", command},
                               {"Raw", raw ? "true" : "false"}})["Value"];
}

auto AlpacaDevice::commandString(const std::string& command,
                                 bool raw) -> std::string {
    return put("commandstring", {{"Command", command},
                                 {"Raw", raw ? "true" : "false"}})["Value"];
}

auto AlpacaDevice::getConnected() -> bool { return get("connected"); }

void AlpacaDevice::setConnected(bool connectedState) {
    put("connected", {{"Connected", connectedState ? "true" : "false"}});
}

auto AlpacaDevice::getDescription() -> std::string {
    return get("description");
}

auto AlpacaDevice::getDriverInfo() -> std::vector<std::string> {
    std::string info = get("driverinfo");
    std::vector<std::string> result;
    std::istringstream iss(info);
    std::string item;
    while (std::getline(iss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

auto AlpacaDevice::getDriverVersion() -> std::string {
    return get("driverversion");
}

auto AlpacaDevice::getInterfaceVersion() -> int {
    return std::stoi(get("interfaceversion").get<std::string>());
}

auto AlpacaDevice::getName() -> std::string { return get("name"); }

auto AlpacaDevice::getSupportedActions() -> std::vector<std::string> {
    return get("supportedactions").get<std::vector<std::string>>();
}

auto AlpacaDevice::get(const std::string& attribute,
                       const std::map<std::string, std::string>& params)
    -> json {
    std::string url = baseUrl_ + "/" + attribute;

    std::string queryString;
    for (const auto& [key, value] : params) {
        if (!queryString.empty()) {
            queryString += "&";
        }
        queryString.append(key).append("=").append(value);
    }

    {
        std::lock_guard<std::mutex> lock(clientTransIdMutex);
        if (!queryString.empty()) {
            queryString += "&";
        }
        queryString += std::format("ClientTransactionID={}&ClientID={}",
                                   clientTransId++, clientId);
    }

    if (!queryString.empty()) {
        url += "?" + queryString;
    }

    std::string responseString;
    curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl_.get());
    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    json response = json::parse(responseString);
    checkError(response);
    return response["Value"];
}

auto AlpacaDevice::put(const std::string& attribute,
                       const std::map<std::string, std::string>& data) -> json {
    std::string url = baseUrl_ + "/" + attribute;

    std::string postFields;
    for (const auto& [key, value] : data) {
        if (!postFields.empty()) {
            postFields += "&";
        }
        postFields.append(key).append("=").append(value);
    }

    {
        std::lock_guard<std::mutex> lock(clientTransIdMutex);
        if (!postFields.empty()) {
            postFields += "&";
        }
        postFields += std::format("ClientTransactionID={}&ClientID={}",
                                  clientTransId++, clientId);
    }

    std::string responseString;
    curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl_.get());
    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    json response = json::parse(responseString);
    checkError(response);
    return response;
}

auto AlpacaDevice::writeCallback(void* contents, size_t size, size_t nmemb,
                                 std::string* s) -> size_t {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

void AlpacaDevice::checkError(const json& response) {
    int errorNumber = response["ErrorNumber"];
    std::string errorMessage = response["ErrorMessage"];

    if (errorNumber != 0) {
        switch (errorNumber) {
            case 0x0400:
                throw std::runtime_error("NotImplementedException: " +
                                         errorMessage);
            case 0x0401:
                throw std::invalid_argument("InvalidValueException: " +
                                            errorMessage);
            case 0x0402:
                throw std::runtime_error("ValueNotSetException: " +
                                         errorMessage);
            case 0x0407:
                throw std::runtime_error("NotConnectedException: " +
                                         errorMessage);
            case 0x0408:
                throw std::runtime_error("ParkedException: " + errorMessage);
            case 0x0409:
                throw std::runtime_error("SlavedException: " + errorMessage);
            case 0x040B:
                throw std::runtime_error("InvalidOperationException: " +
                                         errorMessage);
            case 0x040C:
                throw std::runtime_error("ActionNotImplementedException: " +
                                         errorMessage);
            default:
                if (errorNumber >= 0x500 && errorNumber <= 0xFFF) {
                    throw std::runtime_error(std::format(
                        "DriverException: ({}) {}", errorNumber, errorMessage));
                } else {
                    throw std::runtime_error(
                        std::format("UnknownException: ({}) {}", errorNumber,
                                    errorMessage));
                }
        }
    }
}
