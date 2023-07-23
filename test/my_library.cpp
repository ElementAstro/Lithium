#include "nlohmann/json.hpp"
#include <iostream>
using json = nlohmann::json;

// 示例动态库中的类定义
class MyClass {
public:
    virtual json HandlerFunction(const json& data) {
        std::cout << "Function is called" << std::endl;
        // 处理函数逻辑
        // ...
        return {};
    }
};

// 创建类实例的工厂函数
extern "C" MyClass* CreateInstance() {
    return new MyClass();
}
