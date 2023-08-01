add_executable(toml2json toml2json.cpp)

target_link_libraries(toml2json loguru)

add_executable(ini2json ini2json.cpp)

target_link_libraries(ini2json loguru)

add_executable(json2ini json2ini.cpp)

target_link_libraries(json2ini loguru)