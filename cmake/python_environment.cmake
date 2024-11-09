# Specify the path to requirements.txt
set(REQUIREMENTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt")

# Define a function to check if a Python package is installed
function(check_python_package package version)
    # Replace hyphens with underscores for the import statement
    string(REPLACE "-" "_" import_name ${package})

    # Check if the package can be imported
    execute_process(
        COMMAND ${Python_EXECUTABLE} -c "import ${import_name}"
        RESULT_VARIABLE result
    )

    if(NOT result EQUAL 0)
        set(result FALSE PARENT_SCOPE)
        return()
    endif()

    # Get the installed package version
    execute_process(
        COMMAND ${Python_EXECUTABLE} -m pip show ${package}
        OUTPUT_VARIABLE package_info
    )

    # Extract version information from the output
    string(FIND "${package_info}" "Version:" version_pos)

    if(version_pos EQUAL -1)
        set(result FALSE PARENT_SCOPE)
        return() # Return false if version not found
    endif()

    # Extract the version string
    string(SUBSTRING "${package_info}" ${version_pos} 1000 version_string)
    string(REGEX REPLACE "Version: ([^ ]+).*" "\\1" installed_version "${version_string}")

    # Compare versions
    if("${installed_version}" VERSION_LESS "${version}")
        set(result FALSE PARENT_SCOPE) # Return false if installed version is less than required
        return()
    endif()

    set(result TRUE PARENT_SCOPE)
endfunction()

if (EXISTS "${CMAKE_BINARY_DIR}/check_marker.txt")
    message(STATUS "Check marker file found, skipping the checks.")
else()
# Create a virtual environment
set(VENV_DIR "${CMAKE_BINARY_DIR}/venv")
execute_process(
    COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR}
)

set(PYTHON_EXECUTABLE "${VENV_DIR}/bin/python")
set(PIP_EXECUTABLE "${VENV_DIR}/bin/pip")

# Upgrade pip in the virtual environment
execute_process(
    COMMAND ${PIP_EXECUTABLE} install --upgrade pip
)

# Read the requirements.txt file and install missing packages
file(READ ${REQUIREMENTS_FILE} requirements_content)

# Split the requirements file content into lines
string(REPLACE "\n" ";" requirements_list "${requirements_content}")

# Check and install each package
foreach(requirement ${requirements_list})
    # Skip empty lines
    string(STRIP ${requirement} trimmed_requirement)
    if(trimmed_requirement STREQUAL "")
        continue()
    endif()

    # Get the package name and version (without the version number)
    if(${trimmed_requirement} MATCHES "==")
        string(REPLACE "==" ";" parts ${trimmed_requirement})
    elseif(${trimmed_requirement} MATCHES ">=")
        string(REPLACE ">=" ";" parts ${trimmed_requirement})
    else()
        message(WARNING "Could not parse requirement '${trimmed_requirement}'. Skipping...")
        continue()
    endif()

    list(GET parts 0 package_name)
    list(GET parts 1 package_version)

    # If the package name or version could not be parsed, output a warning and skip
    if(NOT package_name OR NOT package_version)
        message(WARNING "Could not parse requirement '${trimmed_requirement}'. Skipping...")
        continue()
    endif()

    # Check if the package is installed
    message(STATUS "Checking if Python package '${package_name}' is installed...")
    check_python_package(${package_name} ${package_version})
    if(NOT result)
        message(STATUS "Package '${package_name}' is not installed or needs an upgrade. Installing...")
        execute_process(
            COMMAND ${PIP_EXECUTABLE} install ${trimmed_requirement}
            RESULT_VARIABLE install_result
        )
        if(NOT install_result EQUAL 0)
            message(FATAL_ERROR "Failed to install Python package '${package_name}'.")
        endif()
    else()
        message(STATUS "Package '${package_name}' is already installed with a suitable version.")
    endif()
endforeach()
execute_process(
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/check_marker.txt"
    RESULT_VARIABLE result
)
endif()
