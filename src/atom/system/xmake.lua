-- xmake.lua for Atom-System

-- Project Information
set_project("Atom-System")
set_version("1.0.0")
set_description("A collection of useful system functions")
set_licenses("GPL-3.0")

-- Specify the C++ Languages
set_languages("c++17")

-- Add Source Files
add_files("*.cpp")
add_files("module/*.cpp")
add_files("*.hpp")
add_files("module/*.hpp")

-- Add Private Header Files
add_headerfiles("*.hpp", {prefixdir = "$(projectdir)"})
add_headerfiles("module/*.hpp", {prefixdir = "$(projectdir)/module"})

-- Add Target
target("Atom-System")
    set_kind("static")
    add_packages("loguru")
    add_links("pthread")
    add_includedirs(".")
    if is_plat("windows") then
        add_links("pdh", "wlanapi")
    end

-- Install Rules
after_install("install_headers")
after_install("install_libraries")

-- Install Headers
install_headers("*.hpp", "$(projectdir)/include/$(projectname)")
install_headers("module/*.hpp", "$(projectdir)/include/$(projectname)/module")

-- Install Libraries
install_libraries("$(targetdir)/*.a", "$(projectdir)/lib")
