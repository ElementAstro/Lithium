set_project("lithium.config")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Define libraries
local lithium_config_libs = {
    "atom-component",
    "lithium-config",
    "loguru",
    "pthread"
}

-- Sources and Headers
local lithium_config_sources = {
    "_main.cpp",
    "_component.cpp"
}

local lithium_config_headers = {
    "_component.hpp"
}

-- Object Library
target("lithium.config_object")
    set_kind("object")
    add_files(table.unpack(lithium_config_sources))
    add_headerfiles(table.unpack(lithium_config_headers))
    add_packages(table.unpack(lithium_config_libs))
target_end()

-- Shared Library
target("lithium.config")
    set_kind("shared")
    add_deps("lithium.config_object")
    add_files(table.unpack(lithium_config_sources))
    add_headerfiles(table.unpack(lithium_config_headers))
    add_packages(table.unpack(lithium_config_libs))
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()

-- Test Executable
target("lithium.config_test")
    set_kind("binary")
    add_files("_test.cpp")
    add_deps("lithium.config")
    add_packages(table.unpack(lithium_config_libs))
    set_targetdir("$(buildir)/bin")
    if is_mode("debug") then
        add_defines("_DEBUG")
    end
target_end()
