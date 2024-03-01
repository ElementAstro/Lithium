/*
 * exe_plugin.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Executable Plugin

**************************************************/

#pragma once

#include "atom/components/component.hpp"

#include "atom/system/process.hpp"

class ExecutableComponent : public Component
{
public:
    explicit ExecutableComponent(const std::string &name);

    json RunSystemCommand(const json &m_parmas);

    json RunSystemCommandOutput(const json &m_parmas);

    json RunScript(const json &m_parmas);

    json RunScriptOutput(const json &m_parmas);
private:
    std::shared_ptr<Atom::System::ProcessManager> m_ProcessManager;
};
