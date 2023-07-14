#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>
#include "task.hpp"

using namespace Lithium;

// 声明 task_func 函数
nlohmann::json task_func(const nlohmann::json &input);

// Define a function to be executed conditionally
bool print_if_odd(const nlohmann::json &params)
{
    int num = params["number"].get<int>();
    if (num % 2 == 1)
    {
        std::cout << "The number " << num << " is odd." << std::endl;
        return true;
    }
    return false;
}

// Define a condition for ConditionalTask
bool is_greater_than_five(const nlohmann::json &params)
{
    return params["number"].get<int>() > 5;
}

// Define a function to be executed for each item in the list
void print_item(const nlohmann::json &params)
{
    std::cout << "Item: " << params << std::endl;
}

void MyTask()
{
    std::cout << "Hello from daemon task!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main()
{
    // 声明输入参数和任务执行函数
    nlohmann::json input_params;
    std::function<nlohmann::json(const nlohmann::json &)> func = task_func;

    // 创建 SimpleTask 对象并执行
    SimpleTask my_task(func, input_params);
    nlohmann::json output = my_task.Execute();
    // 输出结果
    std::cout << output.dump(4) << std::endl;
    std::cout << my_task.GetResult().dump(4) << std::endl;

    // Create a conditional task
    ConditionalTask ctask(print_if_odd, {{"number", 7}}, is_greater_than_five);

    // Execute the task
    nlohmann::json result = ctask.Execute();

    // Check the result
    if (result["status"] == "done")
    {
        std::cout << "Task executed successfully." << std::endl;
    }
    else
    {
        std::cout << "Task execution failed." << std::endl;
    }

    // Define the list of items to loop over
    nlohmann::json items = {"apple", "banana", "cherry"};

    // Create a loop task
    LoopTask ltask(print_item, {{"items", items}, {"total", items.size()}});

    // Execute the task
    nlohmann::json lresult = ltask.Execute();

    // Check the result
    if (lresult["status"] == "done")
    {
        std::cout << "Task executed successfully." << std::endl;
    }
    else
    {
        std::cout << "Task execution failed." << std::endl;
    }

    DaemonTask dtask(MyTask);
    nlohmann::json dresult = dtask.Execute();
    std::cout << dresult.dump(2) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    dtask.Stop();

    return 0;
}

// 定义 task_func 函数
nlohmann::json task_func(const nlohmann::json &input)
{
    // 处理输入参数并返回输出结果
    nlohmann::json output;
    output["aaa"] = "aaaa";
    // ...
    return output;
}
