/*
 * os.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: OS module for PocketPy(builtin)

**************************************************/

#include "pocketpy/include/pocketpy/bindings.h"

#include "config.h"

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/os.hpp"

using namespace pkpy;

namespace lithium {
void addOSModule(VM* vm) {
    PyObject* mod = vm->new_module("li_os");
    vm->setattr(mod, "version", VAR("1.0.0"));

    vm->bind_func<1>(mod, "walk", [](VM* vm, ArgsView args) {
        std::string_view sv;
        if (is_non_tagged_type(args[0], vm->tp_bytes)) [[likely]] {
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        } else {
            sv = CAST(Str&, args[0]).sv();
        }
        if (!atom::io::isFolderExists(std::string(sv))) {
            LOG_F(ERROR, "Folder is not existing: {}", sv);
            return vm->None;
        }
        auto result = atom::system::jwalk(std::string(sv));
        if (result.empty()) {
            return vm->None;
        }
        return py_var(vm, result);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}
}  // namespace lithium
