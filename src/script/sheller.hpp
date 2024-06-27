/*
 * sheller.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: System Script Manager

**************************************************/

#ifndef LITHIUM_SCRIPT_SHELLER_HPP
#define LITHIUM_SCRIPT_SHELLER_HPP

#include <cstdlib>
#include <ctime>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

using Script = std::string;
#if ENABLE_FASTHASH
using ScriptMap = emhash8::HashMap<std::string, Script>;
#else
using ScriptMap = std::unordered_map<std::string, Script>;
#endif

namespace lithium {
class ScriptManager {
    ScriptMap scripts_;
    std::unordered_map<std::string, Script> powerShellScripts_;
    std::unordered_map<std::string, std::string> scriptOutputs_;
    std::unordered_map<std::string, int> scriptStatus_;
    std::shared_mutex m_sharedMutex_;
    bool registerCommon(std::unordered_map<std::string, std::string>& scriptMap,
                        std::string_view name, const std::string& script);

public:
    void registerScript(std::string_view name, const Script& script);
    void registerPowerShellScript(std::string_view name, const Script& script);
    ScriptMap getAllScripts() const;
    void deleteScript(std::string_view name);
    void updateScript(std::string_view name, const Script& script);
    bool runScript(std::string_view name,
                   const std::unordered_map<std::string, std::string>& args);
};
}  // namespace lithium

#endif