#include <iostream>
#include <dlfcn.h>  // Linux 上的动态库加载头文件
#include "modules/server/commander.hpp"
// JSON 头文件和类型定义，这里使用 nlohmann/json 库作为示例
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class MyClass {
public:
    json HandlerFunction(const json& data) {
        // 处理函数逻辑
        std::cout << "Handling command in dynamic library" << std::endl;
        return json({});
    }
};

int main()
{
    // 加载动态库
    void* libraryHandle = dlopen("./mylibrary.so", RTLD_LAZY);  // 替换为你的动态库路径

    if (libraryHandle == nullptr) {
        std::cout << "Failed to load dynamic library: " << dlerror() << std::endl;
        return 1;
    }

    // 获取类的实例指针
    typedef json (MyClass::*HandlerFunc)(const json&);
    MyClass* instance = nullptr;
    HandlerFunc handler = nullptr;

    // 读取符号（函数）地址
    dlerror();  // 清除之前的错误信息
    *(void**)(&instance) = dlsym(libraryHandle, "CreateInstance");

    const char* dlsymError = dlerror();

    if (dlsymError != nullptr) {
        std::cout << "Failed to find symbol: " << dlsymError << std::endl;
        dlclose(libraryHandle);
        return 1;
    }

    handler = &MyClass::HandlerFunction;

    // 创建 CommandDispatcher 并注册类和函数
    CommandDispatcher dispatcher;
    dispatcher.RegisterHandler("command_name", handler, instance);

    // 使用 CommandDispatcher 派发命令
    json commandData = { /* 命令数据 */ };
    json result = dispatcher.Dispatch("command_name", commandData);

    // 卸载动态库并释放资源
    dlclose(libraryHandle);

    return 0;
}
