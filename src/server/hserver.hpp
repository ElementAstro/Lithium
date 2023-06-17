#pragma once

#include <string>
#include <mutex>
#include "crow/crow.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class HttpServer
{
public:
    HttpServer();
    void RegisterApi();
    void LoginApi();
    void LogoutApi();
    void OnlineUsersApi();
    void StartServer();
    void StopServer();

private:
    std::string EncryptPassword(const std::string &password);
    std::string DecryptPassword(const std::string &cipher_text);

    struct UserInfo
    {
        std::string username;
        std::string password;
    };

    std::vector<UserInfo> user_list_;
    std::vector<std::string> online_users_;
    std::mutex g_mutex_;

    crow::SimpleApp app_;

    std::string key_;
    std::string iv_;
};
