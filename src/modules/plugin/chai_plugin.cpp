#include <chaiscript/chaiscript.hpp>

#include "chai_plugin.hpp"

ChaiScriptPlugin::ChaiScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : Plugin(path, version, author, description)
{
}

void ChaiScriptPlugin::Execute(const std::vector<std::string> &args) const
{
    chaiscript::ChaiScript chai;
    // 加载 ChaiScript 脚本文件
    chai.eval_file(GetPath());
}
