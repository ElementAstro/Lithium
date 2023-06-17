
#include "hserver.hpp"
#include <chrono>
#include <thread>
#include <openssl/evp.h>
#include <openssl/aes.h>

HttpServer::HttpServer() : app_{}
{
    // 设置 AES 密钥和 IV
    key_ = "my_secret_key_123";
    iv_ = "my_initial_vector";
}

void HttpServer::RegisterApi()
{
    CROW_ROUTE(app_, "/register").methods("POST"_method)([&](const crow::request &req) -> crow::response 
                                                         {
            // 从请求中获取用户名和密码
            json reqJson = json::parse(req.body);

                    // Extract username and password from the request JSON
                    std::string username = reqJson["username"].get<std::string>();
                    std::string password = reqJson["password"].get<std::string>();

            // 判断用户名是否已经存在
            bool username_exist = false;
            for (const auto& user_info : user_list_) {
                if (user_info.username == username) {
                    username_exist = true;
                    break;
                }
            }

            if (username_exist) {
                // 用户名已经存在
                json result = {{"status", "error"}, {"message", "username already exists"}};
                return crow::response{ 400, result.dump() };
            } else {
                // 用户名不存在，将新用户加入到用户列表中
                UserInfo new_user_info = {username, EncryptPassword(password)};
                user_list_.push_back(new_user_info);

                json result = {{"status", "ok"}, {"message", "registration successful"}};
                return crow::response{ 200, result.dump() };
            } });
}

void HttpServer::LoginApi()
{
    CROW_ROUTE(app_, "/login").methods("POST"_method)([&](const crow::request &req) -> crow::response 
                                                      {
            json reqJson = json::parse(req.body);

                    // Extract username and password from the request JSON
                    std::string username = reqJson["username"].get<std::string>();
                    std::string password = reqJson["password"].get<std::string>();

            // 遍历用户列表，判断用户名和密码是否匹配
            bool found = false;
            for (const auto& user_info : user_list_) {
                if (user_info.username == username && user_info.password == EncryptPassword(password)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                // 登录成功，将用户加入到在线列表中
                g_mutex_.lock();
                online_users_.push_back(username);
                g_mutex_.unlock();

                json result = {{"status", "ok"}, {"message", "login successful"}};
                return crow::response{ 200, result.dump() };
            } else {
                // 登录失败
                json result = {{"status", "error"}, {"message", "invalid username or password"}};
                return crow::response{ 400, result.dump() };
            } });
}

void HttpServer::LogoutApi()
{
    CROW_ROUTE(app_, "/logout").methods("POST"_method)([&](const crow::request &req) -> crow::response 
                                                       {
            // 从请求中获取用户名
            json reqJson = json::parse(req.body);
                    // Extract username and password from the request JSON
                    std::string username = reqJson["username"].get<std::string>();

            // 遍历在线列表，找到该用户并进行注销
            bool found = false;
            g_mutex_.lock();
            for (auto it = online_users_.begin(); it != online_users_.end(); ++it) {
                if (*it == username) {
                    online_users_.erase(it);
                    found = true;
                    break;
                }
            }
            g_mutex_.unlock();

            if (found) {
                json result = {{"status", "ok"}, {"message", "logout successful"}};
                return crow::response{ 200, result.dump() };
            } else {
                json result = {{"status", "error"}, {"message", "user not logged in"}};
                return crow::response{ 400, result.dump() };
            } });
}

void HttpServer::OnlineUsersApi()
{
    /*
    CROW_ROUTE(app_, "/online-users").methods("GET"_method)([&](crow::response &res)
                                                            {
            // 获取在线用户列表（复制一份出来，避免锁的时间过长）
            std::vector<std::string> online_users;
            g_mutex_.lock();
            online_users = online_users_;
            g_mutex_.unlock();

            // 构造 JSON 响应
            json result = {{"status", "ok"}, {"online_users", online_users}};
            return crow::response{ 200, result.dump() }; });
    */
    
}

void HttpServer::StartServer()
{
    RegisterApi();
    LoginApi();
    LogoutApi();
    OnlineUsersApi();

    app_.port(8080).multithreaded().run();
}

void HttpServer::StopServer()
{
    app_.stop();
}

std::string HttpServer::EncryptPassword(const std::string &password)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("Failed to create cipher context");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char *)key_.c_str(), (unsigned char *)iv_.c_str()) != 1)
    {
        throw std::runtime_error("Failed to initialize cipher context");
    }

    int cipher_text_len = password.size() + AES_BLOCK_SIZE;
    std::string cipher_text(cipher_text_len, '\0');
    int actual_cipher_text_len = 0;

    if (EVP_EncryptUpdate(ctx, (unsigned char *)cipher_text.data(), &cipher_text_len,
                          (unsigned char *)password.c_str(), password.size()) != 1)
    {
        throw std::runtime_error("Failed to encrypt input data");
    }

    if (EVP_EncryptFinal_ex(ctx, (unsigned char *)(cipher_text.data() + cipher_text_len), &actual_cipher_text_len) != 1)
    {
        throw std::runtime_error("Failed to finalize cipher context");
    }

    cipher_text.resize(cipher_text_len + actual_cipher_text_len);
    EVP_CIPHER_CTX_free(ctx);

    return cipher_text;
}

std::string HttpServer::DecryptPassword(const std::string &cipher_text)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("Failed to create cipher context");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char *)key_.c_str(), (unsigned char *)iv_.c_str()) != 1)
    {
        throw std::runtime_error("Failed to initialize cipher context");
    }

    int recovered_text_len = cipher_text.size();
    std::string recovered_text(recovered_text_len, '\0');
    int actual_recovered_text_len = 0;

    if (EVP_DecryptUpdate(ctx, (unsigned char *)recovered_text.data(), &recovered_text_len,
                          (unsigned char *)cipher_text.c_str(), cipher_text.size()) != 1)
    {
        throw std::runtime_error("Failed to decrypt input data");
    }

    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)(recovered_text.data() + recovered_text_len), &actual_recovered_text_len) != 1)
    {
        throw std::runtime_error("Failed to finalize cipher context");
    }

    recovered_text.resize(recovered_text_len + actual_recovered_text_len);
    EVP_CIPHER_CTX_free(ctx);

    return recovered_text;
}