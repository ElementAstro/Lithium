cmake_minimum_required(VERSION 3.20)
project(atom.component C CXX)


list(APPEND ${PROJECT_NAME}_SOURCES
    component.cpp

    templates/alone_component.cpp
    templates/exe_component.cpp
    templates/script_component.cpp
    templates/shared_component.cpp
    templates/task_component.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    component.hpp
    types.hpp

    templates/alone_component.hpp
    templates/exe_component.hpp
    templates/script_component.hpp
    templates/shared_component.hpp
    templates/task_component.hpp
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

add_library(${PROJECT_NAME} STATIC)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC .)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
    SOVERSION ${HYDROGEN_SOVERSION}
    OUTPUT_NAME ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)