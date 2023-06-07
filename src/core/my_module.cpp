#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

#include "modloader.hpp"

using namespace std;
using namespace nlohmann;

json my_function(const json& input) {
    int arg1 = input["arg1"];
    string arg2 = input["arg2"];

    cout << "my_function called with arg1=" << arg1 << ", arg2=" << arg2 << endl;

    json result = {
        {"status", "success"},
        {"result", {{"value", arg1 * 2}, {"text", "hello world"}}}
    };
    return result;
}

class MyTask :public DynamicLoader::Task
{
public:
    json executeTask(const json& input) {
        int arg1 = input["arg1"];
        string arg2 = input["arg2"];

        cout << "MyTask::executeTask called with arg1=" << arg1 << ", arg2=" << arg2 << endl;

        json result = {
            {"status", "success"},
            {"result", {{"value", arg1 * 3}, {"text", "hello dynamic loader"}}}
        };
        return result;
    }
};

extern "C" {
    ::DynamicLoader::FunctionPtr function_name = my_function;
    ::DynamicLoader::Task* class_name = static_cast<::DynamicLoader::Task*>(new MyTask()); // 显式转换为 DynamicLoader::Task*
}

