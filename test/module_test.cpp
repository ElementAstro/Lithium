#include "../src/module/modloader.hpp"
using namespace OpenAPT;

int main()
{
    ModuleLoader loader;

    // 加载模块
    loader.LoadModule("./libexample.so", "example");

    // 获取 Task 类的实例指针
    nlohmann::json taskConfig = {{"param1", 10}, {"param2", true}};
    loader.GetTaskPointer("example", taskConfig)->Execute();
    // 卸载模块
    loader.UnloadModule("example");
    return 0;
}