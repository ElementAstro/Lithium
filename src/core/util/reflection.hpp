#ifndef REFLECTION_H
#define REFLECTION_H
#include <map>
#include <functional>
#include <string>
#include "exception.hpp"

#define LithiumCore_Reflection_VERSION "1.3.0"
#define LithiumCore_Reflection_EDIT_TIME "2023/8/8"
#define LithiumCore_Reflection_AUTHOR "Max Qian"

/*
*   Decoded by utf-8
*   2023/1/2   v-1.1.0 - 初步实现反射机制
*   2023/7/8   v-1.2.0 - 添加宏REGISTER_BY_OTHERNAME,允许自定义注册名称
*   2023/8/8   v-1.3.0  修改命名空间从 LithiumCore 到 LithiumCore::Utilities

*/

// 变量初始化宏
#define INIT                       \
    std::function<void *(void)> f; \
    std::string str_name;

// lambda表达式宏，用于创建类型className的构造回调函数，并通过包装器将其插入到map中

#define REGISTER(className)   \
    f = []() {                \
        return new className; \
    };                        \
    str_name = #className;    \
    this->func_map.insert(pair<std::string, std::function<void *(void)>>(str_name, f));

#define REGISTER_BY_OTHERNAME(className, regist_name) \
    f = []() {                                        \
        return new className;                         \
    };                                                \
    str_name = #regist_name;                          \
    this->func_map.insert(pair<std::string, std::function<void *(void)>>(str_name, f));

namespace LithiumCore
{
    namespace Utilities
    {
        class Reflection;
    };
};

namespace LithiumCore::Utilities
{
    // 抽象类，需要用户去派生
    class Reflection
    {
    protected:
        std::map<std::string, std::function<void *(void)>> func_map;

    public:
        Reflection(){};
        virtual void load() = 0;
        virtual void *createInstance(const char *name_str);
        virtual void *createInstance(const std::string &name_str);
    };
} // namespace LithiumCore
#endif