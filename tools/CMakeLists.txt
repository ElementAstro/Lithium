add_executable(toml2json toml2json.cpp)

target_link_libraries(toml2json loguru)

add_executable(ini2json ini2json.cpp)

target_link_libraries(ini2json loguru)

add_executable(json2ini json2ini.cpp)

target_link_libraries(json2ini loguru)

add_executable(json2xml json2xml.cpp)

target_link_libraries(json2xml loguru)
target_link_libraries(json2xml pugixml-static)

add_executable(xml2json xml2json.cpp)

target_link_libraries(xml2json loguru)
target_link_libraries(xml2json pugixml-static)

add_executable(csv2json csv2json.cpp)
target_link_libraries(csv2json loguru)

include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/io)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/thread)

# ########## getLithium ##############
add_executable(hydrogen_getprop getLithiumproperty.c)

target_link_libraries(hydrogen_getprop hydrogendriverstatic ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})
if(WIN32)
    target_link_libraries(hydrogen_getprop wsock32 ws2_32)
endif()
install(TARGETS hydrogen_getprop RUNTIME DESTINATION bin)

# ########## setLithium ##############
add_executable(hydrogen_setprop setLithiumproperty.c)

target_link_libraries(hydrogen_setprop hydrogendriverstatic ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY})
if(WIN32)
    target_link_libraries(hydrogen_setprop wsock32 ws2_32)
endif()
install(TARGETS hydrogen_setprop RUNTIME DESTINATION bin)
