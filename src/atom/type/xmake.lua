-- xmake.lua for Atom-Type
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Type
-- Description: All of the self-implement types
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom-type")
set_version("1.0.0")
set_license("GPL3")

-- Sources
local sources = {
    "args.cpp",
    "ini.cpp",
    "message.cpp"
}

-- Headers
local headers = {
    "abi.hpp",
    "args.hpp",
    "enum_flag.hpp",
    "enum_flag.inl",
    "flatset.hpp",
    "ini_impl.hpp",
    "ini.hpp",
    "json.hpp",
    "message.hpp",
    "pointer.hpp",
    "small_vector.hpp"
}

-- Build Object Library
target("atom-type-object")
    set_kind("object")
    add_files(headers, {public = true})
    add_files(sources, {public = false})

-- Build Static Library
target("atom-type")
    set_kind("static")
    add_deps("atom-type-object", "atom-utils")
    add_includedirs(".", {public = true})

    set_targetdir("$(buildir)/lib")
    set_objectdir("$(buildir)/obj")

    after_build(function (target)
        os.cp("$(buildir)/lib", "$(projectdir)/lib")
        os.cp("$(projectdir)/*.hpp", "$(projectdir)/include")
    end)