set_project("lithium-task")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru", "pthread")

-- Define libraries
local project_libs = {
    "atom-component",
    "loguru",
    "pthread"
}

-- Source files
local source_files = {
    "container.cpp",
    "generator.cpp",
    "loader.cpp",
    "manager.cpp",
    "singlepool.cpp",
    "task.cpp",
}

-- Header files
local header_files = {
    "container.hpp",
    "generator.hpp",
    "loader.hpp",
    "manager.hpp",
    "pool.hpp",
    "singlepool.hpp",
    "task.hpp"
}

-- Object Library
target("lithium-task_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages(table.unpack(project_libs))
target_end()

-- Static Library
target("lithium-task")
    set_kind("static")
    add_deps("lithium-task_object")
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
