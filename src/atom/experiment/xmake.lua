-- xmake.lua for Atom-Experiment
-- This project is licensed under the terms of the GPL3 license.
--
-- Project Name: Atom-Experiment
-- Description: A collection of experiments for the Atom project
-- Author: Max Qian
-- License: GPL3

add_rules("mode.debug", "mode.release")

set_project("atom-experiment")
set_version("1.0.0")
set_license("GPL3")

-- Sources
local sources = {
    "platform.cpp",
    "string.cpp"
}

-- Headers
local headers = {
    "any.hpp",
    "anyutils.hpp",
    "bind_first.hpp",
    "callable.hpp",
    "decorate.hpp",
    "flatmap.hpp",
    "func_traits.hpp",
    "invoke.hpp",
    "list.hpp",
    "memory.hpp",
    "noncopyable.hpp",
    "object.hpp",
    "optional.hpp",
    "platform.hpp",
    "ranges.hpp",
    "short_alloc.hpp",
    "stack_vector.hpp",
    "static_vector.hpp",
    "string.hpp",
    "to_string.hpp",
    "type_info.hpp"
}

-- Build Object Library
target("atom-experiment-object")
    set_kind("object")
    add_headerfiles(headers, {public = true})
    add_files(sources, {public = false})

-- Build Static Library
target("atom-experiment")
    set_kind("static")
    add_deps("atom-experiment-object")
    add_packages("threads")
    add_includedirs(".", {public = true})

    set_targetdir("$(buildir)/lib")
    set_objectdir("$(buildir)/obj")

    after_build(function (target)
        os.cp("$(buildir)/lib", "$(projectdir)/lib")
        os.cp("$(projectdir)/*.hpp", "$(projectdir)/include")
    end)