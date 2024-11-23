cmake_minimum_required(VERSION 3.20)
project(lithium.cxxtools VERSION 1.0.0)

# Add source files
set(SOURCE_FILES
    src/csv2json.cpp
    src/ini2json.cpp
    src/json2ini.cpp
    src/json2xml.cpp
    src/xml2json.cpp
    src/pci_generator.cpp
    component.cpp
)

# Libraries to link
set(LIBS
    atom-component
    atom-utils
    atom-error
    ${CMAKE_THREAD_LIBS_INIT}
    loguru
    tinyxml2
)

# Create the module library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)