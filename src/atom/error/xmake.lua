set_project("atom-error")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru")

-- Define libraries
local atom_error_libs = {
    "atom-utils"
}

local project_packages = {
    "loguru",
    "dl"
}

-- Source files
local source_files = {
    "error_stack.cpp",
    "exception.cpp",
    "stacktrace.cpp"
}

-- Header files
local header_files = {
    "error_code.hpp",
    "error_stack.hpp",
    "stacktrace.hpp"
}

-- Object Library
target("atom-error_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages("loguru")
    if is_plat("linux") then
        add_syslinks("dl")
    end
target_end()

-- Static Library
target("atom-error")
    set_kind("static")
    add_deps("atom-error_object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_packages("loguru")
    add_deps("atom-utils")
    if is_plat("linux") then
        add_syslinks("dl")
    end
    add_includedirs(".")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
