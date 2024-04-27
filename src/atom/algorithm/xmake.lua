-- xmake.lua for Atom-Algorithm
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Algorithm
-- Description: A collection of algorithms
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom-algorithm")
set_version("1.0.0")
set_license("GPL3")

-- Sources
local sources = {
    "algorithm.cpp",
    "base.cpp",
    "convolve.cpp",
    "fraction.cpp",
    "huffman.cpp",
    "math.cpp",
    "md5.cpp"
}

-- Headers
local headers = {
    "algorithm.hpp",
    "base.hpp",
    "convolve.hpp",
    "fraction.hpp",
    "hash.hpp",
    "huffman.hpp",
    "math.hpp",
    "md5.hpp"
}

-- Build Object Library
target("atom-algorithm-object")
    set_kind("object")
    add_headerfiles(headers, {public = true})
    add_files(sources, {public = false})

-- Build Static Library
target("atom-algorithm")
    set_kind("static")
    add_deps("atom-algorithm-object")
    add_includedirs(".", {public = true})

    set_targetdir("$(buildir)/lib")
    set_objectdir("$(buildir)/obj")

    after_build(function (target)
        os.cp("$(buildir)/lib", "$(projectdir)/lib")
        os.cp("$(projectdir)/*.hpp", "$(projectdir)/include")
    end)

-- Build Python Module (Optional)
if has_config("atom_build_python") then
    target("atom-algorithm-py")
        set_kind("shared")
        add_deps("atom-algorithm")
        add_files("_pybind.cpp")
        add_packages("pybind11")

        after_build(function (target)
            os.cp("$(buildir)/*.so", "$(projectdir)/python")
        end)
end