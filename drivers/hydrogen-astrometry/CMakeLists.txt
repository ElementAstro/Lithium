SET(astrometry_SRC
    astrometrydriver.cpp)

add_executable(hydrogen_astrometry ${astrometry_SRC})
target_link_libraries(hydrogen_astrometry hydrogendriverstatic)
install(TARGETS hydrogen_astrometry RUNTIME DESTINATION bin)