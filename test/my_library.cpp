#include "task.hpp"
#include <memory>
#include <functional>

using namespace Lithium;

// 定义 task_func 函数
nlohmann::json task_func(const nlohmann::json &input)
{
    // 处理输入参数并返回输出结果
    nlohmann::json output;
    output["aaa"] = "aaaa";
    std::cout << "Hello" << std::endl;
    std::cout << "SimpleTask from dynamic lib" << std::endl;
    // ...
    return output;
}

extern "C" std::shared_ptr<SimpleTask> GetTaskInstance(const nlohmann::json&) {
    nlohmann::json input_params;
    std::function<nlohmann::json(const nlohmann::json &)> func = task_func;

    // 创建 SimpleTask 对象并返回
    return std::make_shared<SimpleTask>(func, input_params);
}