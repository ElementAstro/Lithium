set_project("lithium.image")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Add required packages
add_requires("opencv >=4.0", "cfitsio", "loguru", "pthread")

-- Define libraries
local lithium_image_libs = {
    "atom-component",
    "atom-error",
    "opencv",
    "cfitsio",
    "loguru",
    "pthread"
}

-- Sources and Headers
local lithium_image_sources = {
    "_main.cpp",
    "_component.cpp",
    "src/base64.cpp",
    "src/convolve.cpp",
    "src/debayer.cpp",
    "src/fitsio.cpp",
    -- "src/fitskeyword.cpp",
    "src/hfr.cpp",
    "src/hist.cpp",
    "src/stack.cpp",
    "src/stretch.cpp",
    "src/imgutils.cpp"
}

local lithium_image_headers = {
    "include/debayer.hpp",
    "include/fitsio.hpp",
    -- "include/fitskeyword.hpp",
    "include/hfr.hpp",
    "include/hist.hpp",
    "include/stack.hpp",
    "include/stretch.hpp",
    "include/imgutils.hpp"
}

local lithium_image_private_headers = {
    "_component.hpp"
}

-- Object Library
target("lithium.image_object")
    set_kind("object")
    add_files(table.unpack(lithium_image_sources))
    add_headerfiles(table.unpack(lithium_image_headers))
    add_headerfiles(table.unpack(lithium_image_private_headers))
    add_packages(table.unpack(lithium_image_libs))
target_end()

-- Shared Library
target("lithium.image")
    set_kind("shared")
    add_deps("lithium.image_object")
    add_files(table.unpack(lithium_image_sources))
    add_headerfiles(table.unpack(lithium_image_headers))
    add_headerfiles(table.unpack(lithium_image_private_headers))
    add_packages(table.unpack(lithium_image_libs))
    add_includedirs(".", "include", "$(opencv includedirs)")
    set_targetdir("$(buildir)/lib")
    set_installdir("$(installdir)/lib")
    set_version("1.0.0", {build = "%Y%m%d%H%M"})
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
