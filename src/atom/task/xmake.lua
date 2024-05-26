-- xmake.lua for Atom-Task

-- Project Information
set_project("Atom-Task")
set_version("1.0.0")
set_description("Core Task Definitions")
set_licenses("GPL-3.0")

-- Specify the C++ Languages
set_languages("c++17")

-- Add Source Files
add_files("*.cpp")
add_files("*.hpp")

-- Add Target
target("Atom-Task")
    set_kind("static")
    add_links("pthread")
    add_includedirs(".")

-- Install Rules
after_install("install_headers")
after_install("install_libraries")

-- Install Headers
install_headers("*.hpp", "$(projectdir)/include/$(projectname)")

-- Install Libraries
install_libraries("$(targetdir)/*.a", "$(projectdir)/lib")
