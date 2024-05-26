/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#ifndef ATOM_SYSTEM_COMPONENT_HPP
#define ATOM_SYSTEM_COMPONENT_HPP

#include "atom/components/component.hpp"

#include <memory>
#include <unordered_map>

namespace atom::system {
class PidWatcher;
}

class SystemComponent : public Component {
public:
    explicit SystemComponent(const std::string &name);
    ~SystemComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;

protected:
    // -------------------------------------------------------------------
    // Protected methods
    // -------------------------------------------------------------------
    void makePidWatcher(const std::string &name);
    bool startPidWatcher(const std::string &name, const std::string &pid);
    void stopPidWatcher(const std::string &name);
    bool switchPidWatcher(const std::string &name, const std::string &pid);
    void setPidWatcherExitCallback(const std::string &name,
                                   const std::function<void()> &callback);
    void setPidWatcherMonitorFunction(const std::string &name,
                                      const std::function<void()> &callback,
                                      std::chrono::milliseconds interval);
    void getPidByName(const std::string &name, const std::string &pid);

private:
    std::unordered_map<std::string, std::shared_ptr<atom::system::PidWatcher>>
        m_pidWatchers;
};

#endif
