set_project("lithium.client.astap")
set_version("1.0.0")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru", "gtest", "gmock")

-- Define libraries
local project_libs = {
    "atom-system",
    "atom-io",
    "atom-utils",
    "atom-component",
    "atom-error"
}

local project_packages = {
    "loguru"
}

-- Source files
local source_files = {
    "astap.cpp",
    "_component.cpp",
    "_main.cpp"
}

-- Shared Library
target("lithium.client.astap")
    set_kind("shared")
    add_files(table.unpack(source_files))
    add_deps(table.unpack(project_libs))
    add_packages(table.unpack(project_packages))
    add_includedirs("src")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()

-- Test Executable
target("lithium.client.astap.test")
    set_kind("binary")
    add_files("_test.cpp")
    add_deps("lithium.client.astap")
    add_packages("gtest", "gmock")
    set_targetdir("$(buildir)/bin")
target_end()
