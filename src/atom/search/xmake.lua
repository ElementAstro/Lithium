-- xmake.lua for Atom-Search

-- Project Information
set_project("Atom-Search")
set_version("1.0.0")
set_description("Search Engine for Element Astro Project")
set_licenses("GPL-3.0")

-- Specify the C++ Languages
set_languages("c++17")

-- Add Source Files
add_files("*.cpp")
add_files("*.hpp")

-- Add Private Header Files
add_headerfiles("*.hpp", {prefixdir = "$(projectdir)"})

-- Add Target
target("Atom-Search")
    set_kind("static")
    add_packages("loguru")
    add_links("pthread")
    add_includedirs(".")

-- Install Rules
after_install("install_headers")
after_install("install_libraries")

-- Install Headers
install_headers("*.hpp", "$(projectdir)/include/$(projectname)")

-- Install Libraries
install_libraries("$(targetdir)/*.a", "$(projectdir)/lib")
