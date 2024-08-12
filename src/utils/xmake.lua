set_project("lithium-utils")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Source files
local source_files = {
    "constant.cpp"
}

-- Header files
local header_files = {
    "constant.hpp"
}

-- Object Library
target("lithium-utils_object")
    set_kind("object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
target_end()

-- Static Library
target("lithium-utils")
    set_kind("static")
    add_deps("lithium-utils_object")
    add_files(table.unpack(source_files))
    add_headerfiles(table.unpack(header_files))
    add_includedirs(".")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
