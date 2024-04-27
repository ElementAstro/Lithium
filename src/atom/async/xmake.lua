-- xmake.lua for Atom-Async
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Async
-- Description: Async Implementation of Lithium Server and Driver
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

-- Set project name
set_project("atom-async")

-- Set languages
set_languages("cxx17")

-- Set source files
add_files("lock.cpp", "timer.cpp")

-- Set header files
add_headerfiles("*.hpp", "*.inl")

-- Set link libraries
add_linkdirs("path/to/loguru/library")  -- Replace with actual path to loguru library
add_links("loguru")

-- Build static library
target("atom-async")
    set_kind("static")
    add_deps("atom-async-object")
    add_files("lock.cpp", "timer.cpp")
    add_headerfiles("*.hpp", "*.inl")
    add_includedirs(".")
    add_linkdirs(".")
    add_links("loguru")

-- Build object library
target("atom-async-object")
    set_kind("object")
    add_files("lock.cpp", "timer.cpp")
    add_headerfiles("*.hpp", "*.inl")
    add_includedirs(".")
    add_linkdirs(".")
    add_links("loguru")

-- Install target
set_configvar("xmake", "installdir", "/path/to/installation/directory")  -- Replace with actual installation directory
add_installfiles("build/lib/*.a", {prefixdir = "lib"})
