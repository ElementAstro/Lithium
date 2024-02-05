find_package(OpenSSL REQUIRED)
add_library(cpp_httplib httplib.cpp)
target_link_libraries(cpp_httplib OpenSSL::SSL OpenSSL::Crypto)
if(WIN32)
target_link_libraries(cpp_httplib crypt32 wsock32 ws2_32)
endif()
if(LINUX)
    set_target_properties(cpp_httplib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()
