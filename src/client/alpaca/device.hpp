#pragma once

#include <curl/curl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <vector>

#include "atom/function/concept.hpp"

template <typename T>
concept JsonCompatible = requires(nlohmann::json j, T t) {
    { j.get<T>() } -> std::convertible_to<T>;
};

class AlpacaDevice {
public:
    AlpacaDevice(const std::string& address, const std::string& device_type,
                 int device_number, const std::string& protocol);
    virtual ~AlpacaDevice();

    // Common interface methods
    std::string Action(const std::string& ActionName,
                       const std::vector<std::string>& Parameters = {});
    void CommandBlind(const std::string& Command, bool Raw);
    bool CommandBool(const std::string& Command, bool Raw);
    std::string CommandString(const std::string& Command, bool Raw);

    bool GetConnected();
    void SetConnected(bool ConnectedState);
    std::string GetDescription();
    std::vector<std::string> GetDriverInfo();
    std::string GetDriverVersion();
    int GetInterfaceVersion();
    std::string GetName();
    std::vector<std::string> GetSupportedActions();

    template <Number T>
    T GetNumericProperty(const std::string& property_name) {
        return std::get<T>(Get(property_name));
    }

    template <JsonCompatible T>
    std::vector<T> GetArrayProperty(
        const std::string& property,
        const std::map<std::string, std::string>& parameters = {}) {
        nlohmann::json response = Get(property, parameters);
        return response["Value"].get<std::vector<T>>();
    }

protected:
    // HTTP methods
    nlohmann::json Get(const std::string& attribute,
                       const std::map<std::string, std::string>& params = {});
    nlohmann::json Put(const std::string& attribute,
                       const std::map<std::string, std::string>& data = {});

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* s);
    void CheckError(const nlohmann::json& response);

    std::string m_address;
    std::string m_device_type;
    int m_device_number;
    int m_api_version;
    std::string m_base_url;

    static int s_client_id;
    static int s_client_trans_id;
    static std::mutex s_ctid_mutex;

    std::unique_ptr<CURL, std::function<void(CURL*)>> m_curl;
};
