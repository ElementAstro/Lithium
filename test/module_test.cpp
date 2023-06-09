#include "modloader.hpp"
#include "thread.hpp"
using namespace OpenAPT;

int main()
{
    ModuleLoader loader;

    // 加载模块
    loader.LoadModule("libexample.so", "module_name");

    // 获取 Task 类的实例指针
    nlohmann::json taskConfig = {{"param1", 10}, {"param2", true}};
    loader.GetTaskPointer("module_name", taskConfig)->Execute();
    // 卸载模块
    loader.UnloadModule("module_name");
    return 0;
}