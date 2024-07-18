########### Tutorial Six ##############
find_package(INDI 2.0 REQUIRED)
add_executable(indi_test camera.cpp test.cpp)

target_link_libraries(indi_test indiclient loguru ${ZLIB_LIBRARY} ${NOVA_LIBRARIES})
IF (UNIX AND NOT APPLE)
    target_link_libraries(indi_test -lpthread)
ENDIF ()
