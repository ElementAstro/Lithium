set_project("lithium-config")
set_version("1.0.0")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru", "pthread")

-- Define libraries
local project_libs = {
    "loguru",
    "pthread"
}

-- Source files
local source_files = {
    "configor.cpp"
}

-- Header files
local header_files = {
    "configor.hpp"
}

-- Object Library
target("lithium-config_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(project_libs))
target_end()

-- Static Library
target("lithium-config")
    set_kind("static")
    add_deps("lithium-config_object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(project_libs))
    add_includedirs(".")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
