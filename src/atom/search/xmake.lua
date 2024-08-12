-- xmake.lua for Atom-Search

-- Project Information
set_project("Atom-Search")
set_version("1.0.0")
set_description("Search Engine for Element Astro Project")

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
