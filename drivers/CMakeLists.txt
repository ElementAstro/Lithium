include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/io)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/thread)

SET(astrometry_SRC
    hydrogen-astrometry/astrometrydriver.cpp)

add_executable(hydrogen_astrometry ${astrometry_SRC})
target_link_libraries(hydrogen_astrometry hydrogendriverstatic)
install(TARGETS hydrogen_astrometry RUNTIME DESTINATION bin)

add_subdirectory(hydrogen-simulator)

add_subdirectory(hydrogen-weather)

add_subdirectory(hydrogen-ioptron)

add_subdirectory(hydrogen-onstep)
