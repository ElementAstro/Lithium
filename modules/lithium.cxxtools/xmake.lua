set_project("lithium.cxxtools")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Define libraries
local lithium_cxxtools_libs = {
    "atom-component",
    "atom-error",
    "loguru",
    "tinyxml2",
    "pthread"
}

-- Source files
local source_files = {
    "src/csv2json.cpp",
    "src/ini2json.cpp",
    "src/json2ini.cpp",
    "src/json2xml.cpp",
    "src/xml2json.cpp",
    "src/pci_generator.cpp",
    "_component.cpp",
    "_main.cpp"
}

-- Shared Library
target("lithium.cxxtools")
    set_kind("shared")
    add_files(table.unpack(source_files))
    add_packages(table.unpack(lithium_cxxtools_libs))
    add_includedirs("include")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()

-- Add tests subdirectory
includes("tests")
