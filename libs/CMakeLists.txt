add_subdirectory(oatpp)
add_subdirectory(oatpp-swagger)
add_subdirectory(oatpp-websocket)
add_subdirectory(oatpp-openssl)
add_subdirectory(oatpp-zlib)

add_subdirectory(loguru)

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