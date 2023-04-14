#ifndef SCRIPT_MANAGER_H
#define SCRIPT_MANAGER_H

#include <vector>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class ScriptType {
    Sh,
    Ps
};

class ScriptManager {
public:
    explicit ScriptManager(const std::string& path);

    // 遍历给定目录下所有的脚本文件，包括子文件夹
    std::vector<std::string> getScriptFiles() const;

    // 从脚本文件中读取脚本内容
    std::string readScriptFromFile(const std::string& path) const;

    // 校验脚本是否正确
    bool validateScript(const std::string& script, ScriptType scriptType) const;

    // 将脚本名字和路径存储到 JSON 变量中
    json getScriptsJson(const std::vector<std::string>& files) const;

    // 通过名字获取脚本并校验
    bool runScript(const std::string& scriptName, bool async = false) const;

private:
    const std::string m_path;
    const std::vector<std::string> m_files;
    const json m_scriptsJson;

    // 根据文件扩展名判断脚本类型
    ScriptType getScriptType(const std::string& path) const;

    // 根据操作系统不同构建执行脚本的命令
    std::string buildCommand(const std::string& scriptPath) const;

    // 执行命令并获取输出
    std::string executeCommand(const std::string& command) const;
};

#endif // SCRIPT_MANAGER_H
