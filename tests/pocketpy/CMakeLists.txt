add_executable(test-pocketpy bind.cpp)
target_link_libraries(test-pocketpy pocketpy)
target_link_directories(test-pocketpy PUBLIC ${CMAKE_BINARY_DIR}/libs/pocketpy)