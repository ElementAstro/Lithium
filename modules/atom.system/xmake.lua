set_project("atom-system")
set_version("1.0.0")

-- Define libraries
local atom_system_libs = {
    "loguru",
    "atom-component",
    "atom-system",
    "pthread"
}

if is_plat("windows") then
    table.join2(atom_system_libs, {"pdh", "wlanapi", "ws2_32", "userenv", "setupapi", "iphlpapi"})
end

-- Sources and Headers
local atom_system_sources = {
    "_main.cpp",
    "_component.cpp"
}

local atom_system_private_headers = {
    "_component.hpp"
}

-- Object Library
target("atom-system_object")
    set_kind("object")
    add_files(table.unpack(atom_system_sources))
    add_headerfiles(table.unpack(atom_system_private_headers))
    add_packages(table.unpack(atom_system_libs))
target_end()

-- Shared Library
target("atom-system")
    set_kind("shared")
    add_deps("atom-system_object")
    add_files(table.unpack(atom_system_sources))
    add_headerfiles(table.unpack(atom_system_private_headers))
    add_packages(table.unpack(atom_system_libs))
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
