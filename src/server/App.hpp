#ifndef LITHIUM_APP_HPP
#define LITHIUM_APP_HPP

#include "macro.hpp"

namespace lithium {
struct CommandLineArgs {
    int argc;
    const char** argv;
} ATOM_ALIGNAS(16);
auto runServer(CommandLineArgs args) -> bool;
}  // namespace lithium

#endif