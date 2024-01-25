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
    ExecutableComponent();

    void RunSystemCommand(const Args &m_parmas);

    void RunSystemCommandOutput(const Args &m_parmas);

    void RunScript(const Args &m_parmas);

    void RunScriptOutput(const Args &m_parmas);
private:
    std::shared_ptr<Atom::System::ProcessManager> m_ProcessManager;
};
