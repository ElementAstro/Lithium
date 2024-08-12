set_project("lithium.indiserver")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("loguru", "tinyxml2")

-- Define libraries
local packages = {
    "loguru",
    "tinyxml2",
}

local libs = {
    "atom-system",
    "atom-io",
    "atom-component",
    "atom-error"
}

-- Source files
local source_files = {
    "src/indiserver.cpp",
    "src/collection.cpp",
    "src/connector.cpp",
    "_component.cpp",
    "_main.cpp"
}

-- Shared Library
target("lithium.indiserver")
    set_kind("shared")
    add_files(table.unpack(source_files))
    add_deps(table.unpack(libs))
    add_packages(table.unpack(packages))
    add_includedirs("src")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
