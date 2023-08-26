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

# ########## getLithium ##############
#add_executable(lithium_getprop getLithiumproperty.c)

#target_link_libraries(lithium_getprop lithiumcore lithiumdevice dsp eventloop fpack lithiumdriver_OBJECT ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} wsock32 ws2_32)

#install(TARGETS lithium_getprop RUNTIME DESTINATION bin)

# ########## setLithium ##############
#add_executable(lithium_setprop setLithiumproperty.c)

#target_link_libraries(lithium_setprop lithiumcore lithiumdevice dsp eventloop fpack lithiumdriver_OBJECT ${CFITSIO_LIBRARIES} ${NOVA_LIBRARIES} ${M_LIB} ${ZLIB_LIBRARY} wsock32 ws2_32)

#install(TARGETS lithium_setprop RUNTIME DESTINATION bin)
