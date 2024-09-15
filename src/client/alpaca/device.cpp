#include "device.hpp"
#include <format>
#include <sstream>
#include <stdexcept>

int AlpacaDevice::s_client_id = std::random_device{}() % 65536;
int AlpacaDevice::s_client_trans_id = 1;
std::mutex AlpacaDevice::s_ctid_mutex;

AlpacaDevice::AlpacaDevice(const std::string& address,
                           const std::string& device_type, int device_number,
                           const std::string& protocol)
    : m_address(address),
      m_device_type(device_type),
      m_device_number(device_number),
      m_api_version(1) {
    m_base_url = std::format("{}://{}/api/v{}/{}/{}", protocol, address,
                             m_api_version, device_type, device_number);

    m_curl = std::unique_ptr<CURL, std::function<void(CURL*)>>(
        curl_easy_init(), [](CURL* curl) { curl_easy_cleanup(curl); });
    if (!m_curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

AlpacaDevice::~AlpacaDevice() = default;

std::string AlpacaDevice::Action(const std::string& ActionName,
                                 const std::vector<std::string>& Parameters) {
    nlohmann::json params = {{"Action", ActionName},
                             {"Parameters", Parameters}};
    return Put("action", params)["Value"];
}

void AlpacaDevice::CommandBlind(const std::string& Command, bool Raw) {
    Put("commandblind",
        {{"Command", Command}, {"Raw", Raw ? "true" : "false"}});
}

bool AlpacaDevice::CommandBool(const std::string& Command, bool Raw) {
    return Put("commandbool", {{"Command", Command},
                               {"Raw", Raw ? "true" : "false"}})["Value"];
}

std::string AlpacaDevice::CommandString(const std::string& Command, bool Raw) {
    return Put("commandstring", {{"Command", Command},
                                 {"Raw", Raw ? "true" : "false"}})["Value"];
}

bool AlpacaDevice::GetConnected() { return Get("connected"); }

void AlpacaDevice::SetConnected(bool ConnectedState) {
    Put("connected", {{"Connected", ConnectedState ? "true" : "false"}});
}

std::string AlpacaDevice::GetDescription() { return Get("description"); }

std::vector<std::string> AlpacaDevice::GetDriverInfo() {
    std::string info = Get("driverinfo");
    std::vector<std::string> result;
    std::istringstream iss(info);
    std::string item;
    while (std::getline(iss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

std::string AlpacaDevice::GetDriverVersion() { return Get("driverversion"); }

int AlpacaDevice::GetInterfaceVersion() {
    return std::stoi(Get("interfaceversion").get<std::string>());
}

std::string AlpacaDevice::GetName() { return Get("name"); }

std::vector<std::string> AlpacaDevice::GetSupportedActions() {
    return Get("supportedactions").get<std::vector<std::string>>();
}

nlohmann::json AlpacaDevice::Get(
    const std::string& attribute,
    const std::map<std::string, std::string>& params) {
    std::string url = m_base_url + "/" + attribute;

    std::string query_string;
    for (const auto& [key, value] : params) {
        if (!query_string.empty())
            query_string += "&";
        query_string += key + "=" + value;
    }

    {
        std::lock_guard<std::mutex> lock(s_ctid_mutex);
        if (!query_string.empty())
            query_string += "&";
        query_string += std::format("ClientTransactionID={}&ClientID={}",
                                    s_client_trans_id++, s_client_id);
    }

    if (!query_string.empty()) {
        url += "?" + query_string;
    }

    std::string response_string;
    curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(m_curl.get());
    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    nlohmann::json response = nlohmann::json::parse(response_string);
    CheckError(response);
    return response["Value"];
}

nlohmann::json AlpacaDevice::Put(
    const std::string& attribute,
    const std::map<std::string, std::string>& data) {
    std::string url = m_base_url + "/" + attribute;

    std::string post_fields;
    for (const auto& [key, value] : data) {
        if (!post_fields.empty())
            post_fields += "&";
        post_fields += key + "=" + value;
    }

    {
        std::lock_guard<std::mutex> lock(s_ctid_mutex);
        if (!post_fields.empty())
            post_fields += "&";
        post_fields += std::format("ClientTransactionID={}&ClientID={}",
                                   s_client_trans_id++, s_client_id);
    }

    std::string response_string;
    curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl.get(), CURLOPT_POSTFIELDS, post_fields.c_str());
    curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(m_curl.get());
    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::format("CURL error: {}", curl_easy_strerror(res)));
    }

    nlohmann::json response = nlohmann::json::parse(response_string);
    CheckError(response);
    return response;
}

size_t AlpacaDevice::WriteCallback(void* contents, size_t size, size_t nmemb,
                                   std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

void AlpacaDevice::CheckError(const nlohmann::json& response) {
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
