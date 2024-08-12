set_project("atom-io")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Define libraries
local atom_io_libs = {
    "loguru",
    "zlib"
}

-- Sources and Headers
local atom_io_sources = {
    "compress.cpp",
    "file.cpp",
    "io.cpp"
}

local atom_io_headers = {
    "compress.hpp",
    "file.hpp",
    "glob.hpp",
    "io.hpp"
}

-- Object Library
target("atom-io_object")
    set_kind("object")
    add_files(table.unpack(atom_io_sources))
    add_headerfiles(table.unpack(atom_io_headers))
    add_packages(table.unpack(atom_io_libs))
target_end()

-- Static Library
target("atom-io")
    set_kind("static")
    add_deps("atom-io_object")
    add_files(table.unpack(atom_io_sources))
    add_headerfiles(table.unpack(atom_io_headers))
    add_packages(table.unpack(atom_io_libs))
    add_syslinks("pthread")
    set_targetdir("$(buildir)/lib")
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
