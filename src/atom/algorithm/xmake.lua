-- xmake.lua for Atom-Algorithm
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Algorithm
-- Description: A collection of algorithms
-- Author: Max Qian
-- License: GPL3

set_project("atom-algorithm")
set_version("1.0.0") -- Adjust version accordingly

-- Set minimum xmake version
set_xmakever("2.5.1")

-- Define target for object library
target("atom-algorithm_object")
    set_kind("object")
    set_languages("c99", "cxx20")
    set_values("xmake.build.cc.flags", "-fPIC")
    add_files("algorithm.cpp", "base.cpp", "convolve.cpp", "fbase.cpp", "fnmatch.cpp", "fraction.cpp", "huffman.cpp", "math.cpp", "md5.cpp", "mhash.cpp", "pid.cpp")
    add_headerfiles("algorithm.hpp", "algorithm.inl", "base.hpp", "calculator.hpp", "convolve.hpp", "fbase.hpp", "fnmatch.hpp", "fraction.hpp", "hash.hpp", "huffman.hpp", "math.hpp", "md5.hpp", "mhash.hpp", "pid.hpp")
    add_includedirs(".")

-- Define target for static library
target("atom-algorithm")
    set_kind("static")
    set_languages("c99", "cxx20")
    add_deps("atom-algorithm_object")
    add_includedirs(".")
    add_files("algorithm.cpp", "base.cpp", "convolve.cpp", "fbase.cpp", "fnmatch.cpp", "fraction.cpp", "huffman.cpp", "math.cpp", "md5.cpp", "mhash.cpp", "pid.cpp")
    add_headerfiles("algorithm.hpp", "algorithm.inl", "base.hpp", "calculator.hpp", "convolve.hpp", "fbase.hpp", "fnmatch.hpp", "fraction.hpp", "hash.hpp", "huffman.hpp", "math.hpp", "md5.hpp", "mhash.hpp", "pid.hpp")
    add_packages("threads") -- Assuming threads package is needed
    set_values("VERSION", "$(CMAKE_HYDROGEN_VERSION_STRING)")
    set_values("SOVERSION", "$(HYDROGEN_SOVERSION)")

    -- Add installation directory
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)

-- Optionally build Python module if ATOM_BUILD_PYTHON is set
option("ATOM_BUILD_PYTHON")
    set_default(false)
    set_showmenu(true)
    set_description("Build Python bindings")

if has_config("ATOM_BUILD_PYTHON") then
    target("atom-algorithm-py")
        set_kind("shared")
        set_languages("c99", "cxx20")
        add_files("_pybind.cpp")
        add_includedirs(".")
        add_deps("atom-algorithm")
        add_packages("pybind11")
end