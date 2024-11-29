#include "device.hpp"

#include <random>
#include <sstream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

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
        LOG_F(ERROR, "Failed to initialize CURL");
        THROW_CURL_INITIALIZATION_ERROR("Failed to initialize CURL");
    }
    LOG_F(INFO, "AlpacaDevice initialized with base URL: {}", baseUrl_);
}

AlpacaDevice::~AlpacaDevice() {
    LOG_F(INFO, "AlpacaDevice instance destroyed");
}

auto AlpacaDevice::action(const std::string& actionName,
                          const std::vector<std::string>& parameters)
    -> std::string {
    LOG_F(INFO, "Performing action: {} with parameters: {}", actionName, json(parameters).dump());
    json params = {{"Action", actionName}, {"Parameters", parameters}};
    return put("action", params)["Value"];
}

void AlpacaDevice::commandBlind(const std::string& command, bool raw) {
    LOG_F(INFO, "Sending commandBlind: {}, raw: {}", command, raw ? "true" : "false");
    put("commandblind",
        {{"Command", command}, {"Raw", raw ? "true" : "false"}});
}

auto AlpacaDevice::commandBool(const std::string& command, bool raw) -> bool {
    LOG_F(INFO, "Sending commandBool: {}, raw: {}", command, raw ? "true" : "false");
    return put("commandbool", {{"Command", command},
                               {"Raw", raw ? "true" : "false"}})["Value"];
}

auto AlpacaDevice::commandString(const std::string& command,
                                 bool raw) -> std::string {
    LOG_F(INFO, "Sending commandString: {}, raw: {}", command, raw ? "true" : "false");
    return put("commandstring", {{"Command", command},
                                 {"Raw", raw ? "true" : "false"}})["Value"];
}

auto AlpacaDevice::getConnected() -> bool {
    LOG_F(INFO, "Getting connected state");
    return get("connected");
}

void AlpacaDevice::setConnected(bool connectedState) {
    LOG_F(INFO, "Setting connected state to: {}", connectedState ? "true" : "false");
    put("connected", {{"Connected", connectedState ? "true" : "false"}});
}

auto AlpacaDevice::getDescription() -> std::string {
    LOG_F(INFO, "Getting description");
    return get("description");
}

auto AlpacaDevice::getDriverInfo() -> std::vector<std::string> {
    LOG_F(INFO, "Getting driver info");
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
    LOG_F(INFO, "Getting driver version");
    return get("driverversion");
}

auto AlpacaDevice::getInterfaceVersion() -> int {
    LOG_F(INFO, "Getting interface version");
    return std::stoi(get("interfaceversion").get<std::string>());
}

auto AlpacaDevice::getName() -> std::string {
    LOG_F(INFO, "Getting name");
    return get("name");
}

auto AlpacaDevice::getSupportedActions() -> std::vector<std::string> {
    LOG_F(INFO, "Getting supported actions");
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

    LOG_F(INFO, "Sending GET request to URL: {}", url);

    std::string responseString;
    curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl_.get());
    if (res != CURLE_OK) {
        LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(res));
        THROW_RUNTIME_ERROR(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    LOG_F(INFO, "Received response: {}", responseString);

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

    LOG_F(INFO, "Sending PUT request to URL: {} with data: {}", url, postFields);

    std::string responseString;
    curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl_.get());
    if (res != CURLE_OK) {
        LOG_F(ERROR, "CURL error: {}", curl_easy_strerror(res));
        THROW_RUNTIME_ERROR(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    LOG_F(INFO, "Received response: {}", responseString);

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
        LOG_F(ERROR, "Memory allocation error in writeCallback");
        return 0;
    }
    return newLength;
}

void AlpacaDevice::checkError(const json& response) {
    int errorNumber = response["ErrorNumber"];
    std::string errorMessage = response["ErrorMessage"];

    if (errorNumber != 0) {
        LOG_F(ERROR, "Error received: %d - {}", errorNumber, errorMessage);
        switch (errorNumber) {
            case 0x0400:
                THROW_RUNTIME_ERROR("NotImplementedException: " +
                                         errorMessage);
            case 0x0401:
                THROW_INVALID_ARGUMENT("InvalidValueException: " +
                                            errorMessage);
            case 0x0402:
                THROW_RUNTIME_ERROR("ValueNotSetException: " +
                                         errorMessage);
            case 0x0407:
                THROW_RUNTIME_ERROR("NotConnectedException: " +
                                         errorMessage);
            case 0x0408:
                THROW_RUNTIME_ERROR("ParkedException: " + errorMessage);
            case 0x0409:
                THROW_RUNTIME_ERROR("SlavedException: " + errorMessage);
            case 0x040B:
                THROW_RUNTIME_ERROR("InvalidOperationException: " +
                                         errorMessage);
            case 0x040C:
                THROW_RUNTIME_ERROR("ActionNotImplementedException: " +
                                         errorMessage);
            default:
                if (errorNumber >= 0x500 && errorNumber <= 0xFFF) {
                    THROW_RUNTIME_ERROR(std::format(
                        "DriverException: ({}) {}", errorNumber, errorMessage));
                } else {
                    THROW_RUNTIME_ERROR(
                        std::format("UnknownException: ({}) {}", errorNumber,
                                    errorMessage));
                }
        }
    }
}