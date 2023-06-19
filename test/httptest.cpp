#include "httpclient.hpp"
#include <iostream>

int main()
{
    HttpClient client("localhost");

    // 发送 GET 请求
    json response;
    std::string err;
    std::map<std::string, std::string> params = {{"param1", "value1"}, {"param2", "value2"}};
    if (client.SendGetRequest("/api/get_data", params, response, err))
    {
        std::cout << "Response: " << response.dump() << std::endl;
    } else {
        std::cerr << "Failed to get data: " << err << std::endl;
    }

    // 发送 POST 请求
    json data = {{"key1", "value1"}, {"key2", "value2"}};
    if (client.SendPostRequest("/api/add_data", params, data, response, err))
    {
        std::cout << "Response: " << response.dump() << std::endl;
    } else {
        std::cerr << "Failed to add data: " << err << std::endl;
    }

    // 扫描端口
    std::vector<int> open_ports;
    if (client.ScanPort(80, 90, open_ports))
    {
        for (auto& port : open_ports)
        {
            std::cout << "Port " << port << " is open on example.com" << std::endl;
        }
    } else {
        std::cerr << "Failed to scan ports" << std::endl;
    }

    // 检查服务器状态
    std::string server_status;
    if (client.CheckServerStatus(server_status))
    {
        std::cout << "Server status: " << server_status << std::endl;
    } else {
        std::cerr << "Failed to check server status: " << server_status << std::endl;
    }

    return 0;
}
