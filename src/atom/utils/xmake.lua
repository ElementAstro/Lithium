-- xmake.lua for Atom-Utils
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Utils
-- Description: A collection of useful functions
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom-utils")
set_version("1.0.0")
set_license("GPL3")

-- Sources
local sources = {
    "aes.cpp",
    "env.cpp",
    "hash_util.cpp",
    "random.cpp",
    "string.cpp",
    "stopwatcher.cpp",
    "time.cpp",
    "uuid.cpp",
    "xml.cpp"
}

-- Headers
local headers = {
    "aes.hpp",
    "env.hpp",
    "hash_util.hpp",
    "random.hpp",
    "refl.hpp",
    "string.hpp",
    "stopwatcher.hpp",
    "switch.hpp",
    "time.hpp",
    "uuid.hpp",
    "xml.hpp"
}

-- Private Headers
local private_headers = {
}

-- Build Object Library
target("atom-utils-object")
    set_kind("object")
    add_files(headers, {public = true})
    add_files(sources, private_headers, {public = false})
    add_packages("loguru", "tinyxml2")

-- Build Static Library
target("atom-utils")
    set_kind("static")
    add_deps("atom-utils-object")
    add_packages("loguru", "tinyxml2")
    add_includedirs(".", {public = true})

    set_targetdir("$(buildir)/lib")
    set_objectdir("$(buildir)/obj")

    after_build(function (target)
        os.cp("$(buildir)/lib", "$(projectdir)/lib")
        os.cp("$(projectdir)/*.hpp", "$(projectdir)/include")
    end)