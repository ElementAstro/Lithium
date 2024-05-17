-- xmake.lua for Atom-IO
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-IO
-- Description: IO Components for Element Astro Project
-- Author: Max Qian
-- License: GPL3

set_project("atom-io")
set_version("1.0.0") -- Adjust version accordingly

-- Set minimum xmake version
set_xmakever("2.5.1")

-- Define target
target("atom-io")
    set_kind("static")
    set_languages("c99", "cxx20")

    -- Add include directories
    add_includedirs(".")

    -- Add source files
    add_files("compress.cpp", "file.cpp", "io.cpp")

    -- Add header files
    add_headerfiles("compress.hpp", "file.hpp", "glob.hpp", "io.hpp")

    -- Add dependencies
    add_packages("loguru", "libzippp", {public = true})

    -- Set position independent code
    set_values("xmake.build.cc.flags", "-fPIC")

    -- Set version and soversion
    set_configvar("VERSION", "$(CMAKE_HYDROGEN_VERSION_STRING)")
    set_configvar("SOVERSION", "$(HYDROGEN_SOVERSION)")

    -- Add installation directory
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
