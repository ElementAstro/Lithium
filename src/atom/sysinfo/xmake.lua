set_project("atom-sysinfo")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru")

-- Define libraries
local atom_sysinfo_libs = {
    "loguru",
    "pthread"
}

-- Source files
local source_files = {
    "battery.cpp",
    "cpu.cpp",
    "disk.cpp",
    "gpu.cpp",
    "memory.cpp",
    "os.cpp",
    "wifi.cpp"
}

-- Header files
local header_files = {
    "battery.hpp",
    "cpu.hpp",
    "disk.hpp",
    "gpu.hpp",
    "memory.hpp",
    "os.hpp",
    "wifi.hpp"
}

-- Object Library
target("atom-sysinfo_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(atom_sysinfo_libs))
target_end()

-- Static Library
target("atom-sysinfo")
    set_kind("static")
    add_deps("atom-sysinfo_object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(atom_sysinfo_libs))
    if is_plat("windows") then
        add_syslinks("pdh", "wlanapi")
    end
    add_includedirs(".")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
        for _, header in ipairs(header_files) do
            os.cp(header, path.join(target:installdir(), "include/atom-sysinfo"))
        end
    end)
target_end()
