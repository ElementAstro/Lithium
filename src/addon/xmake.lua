
set_project("lithium-addons")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru", "pthread")

local project_packages = {
    "loguru",
    "pthread"
}

-- Define libraries
local project_libs = {
    "atom-io",
    "atom-error",
    "atom-function",
    "atom-system",
    "lithium-utils"
}

-- Source files
local source_files = {
    "addons.cpp",
    "compiler.cpp",
    "dependency.cpp",
    "loader.cpp",
    "manager.cpp",
    "sandbox.cpp"
}

-- Header files
local header_files = {
    "addons.hpp",
    "compiler.hpp",
    "dependency.hpp",
    "loader.hpp",
    "manager.hpp",
    "sandbox.hpp"
}

-- Object Library
target("lithium-addons_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_deps(table.unpack(project_libs))
    add_packages(table.unpack(project_packages))
target_end()

-- Static Library
target("lithium-addons")
    set_kind("static")
    add_deps("lithium-addons_object")
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
