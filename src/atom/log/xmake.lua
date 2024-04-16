-- xmake.lua for Loguru
-- This project is licensed under the terms of the MIT license.

add_rules("mode.debug", "mode.release")

-- Define project and policies
set_project("loguru")
set_version("2.1.0")
set_languages("cxx11")

-- Expose xmake-specific user options
option("build_examples", {showmenu = true, default = false, description = "Build the project examples"})
option("build_tests", {showmenu = true, default = false, description = "Build the tests"})

-- Set global compile flags
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
else
    set_symbols("hidden")
    set_optimize("fastest")
    set_strip("all")
end

-- Add loguru target
target("loguru")
    set_kind("$(kind)")
    add_files("loguru.cpp")
    add_headerfiles("loguru.hpp")
    add_includedirs(".", {public = true})
    set_configvar("LOGURU_STACKTRACES", 1)
    set_configvar("LOGURU_USE_FMTLIB", 1)

    -- Determine if linking 'dl' is required
    if is_plat("windows") then
        add_packages("dlfcn-win32")
        add_links("dlfcn-win32", "dbghelp")
    else
        add_links("dl")
    end

    -- Link fmt (if needed)
    if has_config("LOGURU_USE_FMTLIB") then
        add_packages("fmt")
        if has_config("LOGURU_FMT_HEADER_ONLY") then
            add_packages("fmt", {configs = {header_only = true}})
        end
    end

    -- Set target properties
    set_configvar("LOGURU_DEBUG_LOGGING", 0)
    set_configvar("LOGURU_DEBUG_CHECKS", 0)
    set_configvar("LOGURU_REDEFINE_ASSERT", 0)
    set_configvar("LOGURU_WITH_STREAMS", 1)
    set_configvar("LOGURU_REPLACE_GLOG", 0)
    set_configvar("LOGURU_FMT_HEADER_ONLY", 0)
    set_configvar("LOGURU_WITH_FILEABS", 0)
    set_configvar("LOGURU_RTTI", 1)
    set_configvar("LOGURU_FILENAME_WIDTH", 23)
    set_configvar("LOGURU_THREADNAME_WIDTH", 16)
    set_configvar("LOGURU_SCOPE_TIME_PRECISION", 3)
    set_configvar("LOGURU_VERBOSE_SCOPE_ENDINGS", 0)

-- Setup examples
if has_config("build_examples") then
    -- TODO: Add example targets
end

-- Setup tests
if has_config("build_tests") then
    -- TODO: Add test targets
end

-- Setup install rules
target("install")
    set_kind("phony")
    on_install(function (target)
        -- Install the main library
        install_targets("loguru", {libdir = "lib"})

        -- Install the header file
        install_headers("loguru.hpp", {prefixdir = "include/loguru"})

        -- Install pkgconfig file
        -- TODO: Generate and install pkgconfig file
    end)