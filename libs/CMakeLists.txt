add_subdirectory(oatpp)
add_subdirectory(oatpp-swagger)
add_subdirectory(oatpp-websocket)
add_subdirectory(oatpp-openssl)
add_subdirectory(oatpp-zlib)

add_library(loguru loguru/loguru.cpp)
if(LINUX)
    find_package(Threads REQUIRED)
    target_link_libraries(loguru Threads::Threads dl)
    set_target_properties(loguru PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()

find_package(OpenSSL REQUIRED)
add_library(cpp_httplib cpp_httplib/httplib.cpp)
target_link_libraries(cpp_httplib OpenSSL::SSL OpenSSL::Crypto)
if(WIN32)
target_link_libraries(cpp_httplib crypt32 wsock32 ws2_32)
endif()
if(LINUX)
    set_target_properties(cpp_httplib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()


add_subdirectory(backward)

add_subdirectory(pugixml)

add_subdirectory(libzippp)