SET(astap_SRC
    astapdriver.cpp)

add_executable(hydrogen_astap ${astap_SRC})
target_link_libraries(hydrogen_astap hydrogendriverstatic)
install(TARGETS hydrogen_astap RUNTIME DESTINATION bin)