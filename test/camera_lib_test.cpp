#include "../src/modules/device/camera.hpp"

#include "nlohmann/json.hpp"

extern "C" std::shared_ptr<Camera> GetInstance(const nlohmann::json& params) {
    std::string name;
    if (!params.contains("name"))
    {
        name = "MyCamera";
    }
    else
    {
        name = params["name"];
    }
    // 创建 SimpleTask 对象并返回
    return std::make_shared<Camera>("MyCamera");
}