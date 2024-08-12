set_project("atom-component")
set_version("1.0.0")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru")

-- Define libraries
local atom_component_libs = {
    "atom-error",
    "atom-type",
    "atom-utils"
}

local atom_component_packages = {
    "loguru",
    "pthread"
}

-- Source files
local source_files = {
    "registry.cpp"
}

-- Header files
local header_files = {
    "component.hpp",
    "dispatch.hpp",
    "types.hpp",
    "var.hpp"
}

-- Object Library
target("atom-component_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_deps(table.unpack(atom_component_libs))
    add_packages(table.unpack(atom_component_packages))
target_end()

-- Static Library
target("atom-component")
    set_kind("static")
    add_deps("atom-component_object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(atom_component_libs))
    add_includedirs(".")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
