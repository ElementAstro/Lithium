set_project("atom.sysinfo")
set_version("1.0.0")
set_xmakever("2.5.1")

-- Set the C++ standard
set_languages("cxx20")

-- Source files
local source_files = {
    "_component.cpp",
    "_main.cpp"
}

-- Shared Library
target("atom.sysinfo")
    set_kind("shared")
    add_files(table.unpack(source_files))
    add_includedirs("include")
    set_targetdir("$(buildir)/lib")
    on_install(function (target)
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end)
target_end()
