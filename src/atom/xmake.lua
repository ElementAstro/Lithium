-- xmake.lua for Atom
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom
-- Description: Atom Library for all of the Element Astro Project
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom")
set_version("1.0.0")
set_license("GPL3")

option("atom_build_python", {description = "Build Atom with Python support", default = false})

if has_config("atom_build_python") then
    add_requires("python 3.x", {kind = "binary"})
    add_requires("pybind11")
end

add_subdirs("algorithm", "async", "components", "connection", "driver", "event", "experiment", "io", "log", "server", "search", "system", "task", "type", "utils", "web")

if not has_config("HAS_STD_FORMAT") then
    add_requires("fmt")
end

-- Sources
local sources = {
    "error/error_stack.cpp",
    "log/logger.cpp",
    "log/global_logger.cpp",
    "log/syslog.cpp"
}

-- Headers
local headers = {
    "error/error_code.hpp",
    "error/error_stack.hpp",
    "log/logger.hpp",
    "log/global_logger.hpp",
    "log/syslog.hpp"
}

-- Private Headers
local private_headers = {
}

-- Dependencies
local dependencies = {
    "loguru",
    "cpp_httplib",
    "libzippp",
    "atom-async",
    "atom-task",
    "atom-io",
    "atom-driver",
    "atom-event",
    "atom-experiment",
    "atom-component",
    "atom-type",
    "atom-utils",
    "atom-search",
    "atom-web",
    "atom-system",
    "atom-server"
}

-- Build Object Library
target("atom-object")
    set_kind("object")
    add_files(headers, {public = true})
    add_files(sources, private_headers, {public = false})
    add_defines("HAVE_LIBNOVA")
    add_packages(dependencies)

    if is_plat("windows") then
        add_syslinks("setupapi", "wsock32", "ws2_32", "shlwapi", "iphlpapi")
    end

-- Build Static Library
target("atom")
    set_kind("static")
    add_deps("atom-object")
    add_packages(dependencies)
    add_includedirs(".", {public = true})

    set_targetdir("$(buildir)/lib")
    set_objectdir("$(buildir)/obj")

    after_build(function (target)
        os.cp("$(buildir)/lib", "$(projectdir)/lib")
        os.cp("$(projectdir)/*.hpp", "$(projectdir)/include")
    end)