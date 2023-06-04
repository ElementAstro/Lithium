cmake_minimum_required(VERSION 3.13)
project(indiserver CXX)

if(WIN32 OR ANDROID)
    add_subdirectory(indicore)
        
    find_package(Threads REQUIRED)
    find_package(Libev REQUIRED)

    add_executable(${PROJECT_NAME} indiserver.cpp indiserver.hpp indidvrinfo.cpp indidvrinfo.hpp indimsg.cpp indimsg.hpp indismsg.cpp indismsg.hpp)

    target_precompile_headers(indiserver PUBLIC indiserver.hpp)

    target_link_libraries(indiserver indicore ${CMAKE_THREAD_LIBS_INIT} ${LIBEV_LIBRARIES})
    target_include_directories(indiserver SYSTEM PRIVATE ${LIBEV_INCLUDE_DIRS})

    find_package(spdlog REQUIRED)
    target_link_libraries(indiserver spdlog::spdlog_header_only)

    install(TARGETS indiserver RUNTIME DESTINATION bin)

    set_target_properties(
        indiserver
        PROPERTIES
        OUTPUT_NAME ../../indiserver
    )
else()

    add_subdirectory(indicore)
    
    find_package(Threads REQUIRED)
    find_package(Libev REQUIRED)

    add_executable(${PROJECT_NAME} indiserver.cpp indiserver.hpp indidvrinfo.cpp indidvrinfo.hpp indimsg.cpp indimsg.hpp indismsg.cpp indismsg.hpp)

    target_precompile_headers(indiserver PUBLIC indiserver.hpp)

    target_link_libraries(indiserver indicore ${CMAKE_THREAD_LIBS_INIT} ${LIBEV_LIBRARIES})
    target_include_directories(indiserver SYSTEM PRIVATE ${LIBEV_INCLUDE_DIRS})

    find_package(spdlog REQUIRED)
    target_link_libraries(indiserver spdlog::spdlog_header_only)

    install(TARGETS indiserver RUNTIME DESTINATION bin)

    set_target_properties(
        indiserver
        PROPERTIES
        OUTPUT_NAME ../../indiserver
    )
endif(WIN32 OR ANDROID)
