set_project("atom-utils")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Define libraries
local atom_utils_libs = {
    "loguru",
    "atom-component",
    "atom-utils",
    "openssl",
    "pthread"
}

-- Sources and Headers
local atom_utils_sources = {
    "_main.cpp",
    "_component.cpp"
}

local atom_utils_private_headers = {
    "_component.hpp"
}

-- Object Library
target("atom-utils_object")
    set_kind("object")
    add_files(table.unpack(atom_utils_sources))
    add_headerfiles(table.unpack(atom_utils_private_headers))
    add_packages(table.unpack(atom_utils_libs))
target_end()

-- Shared Library
target("atom-utils")
    set_kind("shared")
    add_deps("atom-utils_object")
    add_files(table.unpack(atom_utils_sources))
    add_headerfiles(table.unpack(atom_utils_private_headers))
    add_packages(table.unpack(atom_utils_libs))
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
