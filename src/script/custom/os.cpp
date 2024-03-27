#include "pocketpy/include/pocketpy/bindings.h"

#include "config.h"

#include "atom/system/os.hpp"
#include "atom/io/io.hpp"

using namespace pkpy;
void addOSModule(VM* vm){
    PyObject* mod = vm->new_module("li_os");
    vm->setattr(mod, "version", VAR(LITHIUM_VERSION_STRING));

    vm->bind_func<1>(mod, "walk", [](VM* vm, ArgsView args) {
        std::string_view sv;
        if(is_non_tagged_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        if (!Atom::IO::isisFolderExists(sv))
        {
            
        }
        auto result = Atom::System::jwalk(sv);
        if (result.empty()){
            return vm->None;
        }
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}