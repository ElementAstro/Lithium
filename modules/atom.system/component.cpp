#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/system/command.hpp"
#include "atom/system/crash.hpp"
#include "atom/system/pidwatcher.hpp"
#include "atom/system/platform.hpp"
#include "atom/system/user.hpp"

#include "atom/log/loguru.hpp"

using namespace atom::system;

ATOM_MODULE(atom_system, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    DLOG_F(INFO, "Loaded module {}", component.getName());
});
