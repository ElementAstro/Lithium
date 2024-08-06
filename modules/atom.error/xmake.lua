set_project("atom-error")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Define libraries
local atom_error_libs = {
    "loguru",
    "atom-component",
    "atom-error",
    "zlib"
}

-- Sources and Headers
local atom_error_sources = {
    "_main.cpp",
    "_component.cpp"
}

local atom_error_private_headers = {
    "_component.hpp"
}

-- Object Library
target("atom-error_object")
    set_kind("object")
    add_files(table.unpack(atom_error_sources))
    add_headerfiles(table.unpack(atom_error_private_headers))
    add_packages(table.unpack(atom_error_libs))
target_end()

-- Shared Library
target("atom-error")
    set_kind("shared")
    add_deps("atom-error_object")
    add_files(table.unpack(atom_error_sources))
    add_headerfiles(table.unpack(atom_error_private_headers))
    add_packages(table.unpack(atom_error_libs))
    add_syslinks("pthread")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    set_targetdir("$(buildir)/lib")
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
