set_project("lithium-client")
set_version("1.0.0")

-- Set the C++ standard
set_languages("cxx20")

-- Function to add subdirectories recursively
function add_subdirectories_recursively(start_dir)
    local dirs = os.dirs(path.join(start_dir, "*"))
    for _, dir in ipairs(dirs) do
        if os.isfile(path.join(dir, "xmake.lua")) then
            includes(dir)
        end
    end
end

-- Call the function to add subdirectories recursively
add_subdirectories_recursively(".")
