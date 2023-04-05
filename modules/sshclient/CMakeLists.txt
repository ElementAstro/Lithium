set(${CMAKE_LIBRARY_PATH} ${CMAKE_BINARY_DIR})

find_package(OpenSSL REQUIRED)

add_library(sshclient SHARED sshclient.cpp)