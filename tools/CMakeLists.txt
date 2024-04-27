find_package(tomlplusplus)

if (tomlplusplus_FOUND)
    add_executable(toml2json toml2json.cpp)
    target_link_libraries(toml2json loguru)
endif()

add_executable(ini2json ini2json.cpp)
target_link_libraries(ini2json loguru)

add_executable(json2ini json2ini.cpp)
target_link_libraries(json2ini loguru)

add_executable(json2xml json2xml.cpp)
target_link_libraries(json2xml loguru)
target_link_libraries(json2xml tinyxml2)

add_executable(xml2json xml2json.cpp)
target_link_libraries(xml2json loguru)
target_link_libraries(xml2json tinyxml2)

add_executable(csv2json csv2json.cpp)
target_link_libraries(csv2json loguru)

add_executable(tcp_proxy tcp_proxy.cpp)
target_link_libraries(tcp_proxy loguru)
if(WIN32)
target_link_libraries(tcp_proxy ws2_32)
endif()
