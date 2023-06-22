#include "sheller.hpp"
#include <iostream>

int main()
{
    // 创建 ScriptManager 对象
    ScriptManager scriptManager;

    // 添加脚本
    scriptManager.AddScript("script1", "scripts/script1.py", 1, "arg1 arg2", "string string");
    scriptManager.AddScript("script2", "scripts/script2.sh", 2, "100 200", "int int");

    // 获取所有脚本名称
    std::vector<std::string> scriptNames = scriptManager.GetScriptNames();
    std::cout << "All scripts: ";
    for (const auto &name : scriptNames)
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    // 获取脚本信息
    std::map<std::string, std::string> scriptInfo1 = scriptManager.GetScript("script1");
    std::cout << "Script1 info:\n";
    for (const auto &[key, value] : scriptInfo1)
    {
        std::cout << key << ": " << value << "\n";
    }

    // 运行脚本
    std::string output = scriptManager.RunScript("script2", {"100", "100"});
    std::cout << "Script2 output:\n"
              << output << std::endl;

    return 0;
}
