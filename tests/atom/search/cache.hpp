#include "atom/search/cache.hpp"

#include <iostream>

int main()
{
    ResourceCache<json> jsonCache(1000);
    jsonCache.readFromJsonFile("data.json", [](const json &data)
                               { return data; });

    // 从缓存中获取数据
    try
    {
        const json &jsonData = jsonCache.get("M1");
        std::cout << "Data for M1: " << jsonData.dump(4) << std::endl;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << e.what() << std::endl;
    }

    jsonCache.insert("M2", json{{"name", "M2"}, {"age", 25}}, std::chrono::seconds(60));
    jsonCache.asyncLoad("M3", []()
                        {
        // 模拟异步加载数据的耗时操作
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return json{{"name", "M3"}, {"age", 30}}; });

    try
    {
        auto jsonData = jsonCache.asyncGet("M3").get();
        std::cout << "Data for M3: " << jsonData.dump(4) << std::endl;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << e.what() << std::endl;
    }

    try
    {
        const json &jsonData = jsonCache.get("M2");
        std::cout << "Data for M2: " << jsonData.dump(4) << std::endl;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << e.what() << std::endl;
    }

    // 将缓存内容写入文件
    jsonCache.writeToJsonFile("updated_data.json", [](const json &data)
                              { return data; });

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
