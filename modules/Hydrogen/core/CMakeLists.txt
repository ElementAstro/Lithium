cmake_minimum_required(VERSION 3.13)
project(hydrogendriver C CXX)

find_package(USB1 REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)
include_directories(${USB1_INCLUDE_DIRS})

add_subdirectory(io)

include_directories(.)
include_directories(timer)
include_directories(thread)
include_directories(stream)
include_directories(base)
include_directories(property)

add_subdirectory(base)

# Dependency
find_package(FFTW3 REQUIRED)

list(APPEND ${PROJECT_NAME}_LIBS
    hydrogencore
    hydrogendevice
    eventloop
    dsp
    fpack
    hid
    ${USB1_LIBRARIES}
    ${NOVA_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CFITSIO_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${JPEG_LIBRARY}
    ${FFTW3_LIBRARIES}
    ${M_LIB}
    $<$<PLATFORM_ID:CYGWIN>:${ICONV_LIBRARIES}>
)

# Add Iconv
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    # FreeBSD needs to find the correct GNU iconv library.
    find_package(Iconv REQUIRED)
    list(APPEND ${PROJECT_NAME}_LIBS ${ICONV_LIBRARIES})
endif()

# Add LibXISF
find_package(LibXISF)
if(LibXISF_FOUND)
    list(APPEND ${PROJECT_NAME}_LIBS ${LibXISF_LIBRARY})
    include_directories(${LibXISF_INCLUDE_DIR})
    add_definitions(-DHAVE_XISF)
endif()

# Add OggTheora, StreamManager, v4l2
if(UNIX OR WIN32)
    list(APPEND ${PROJECT_NAME}_SOURCES
        stream/streammanager.cpp
        stream/fpsmeter.cpp
        stream/gammalut16.cpp
        stream/recorder/recorderinterface.cpp
        stream/recorder/recordermanager.cpp
        stream/recorder/serrecorder.cpp
        stream/encoder/encodermanager.cpp
        stream/encoder/encoderinterface.cpp
        stream/encoder/rawencoder.cpp
        stream/encoder/mjpegencoder.cpp
        stream/jpegutils.c
        stream/ccvt_c2.c
        stream/ccvt_misc.c
    )

    install(FILES
        stream/streammanager.h
        stream/fpsmeter.h
        stream/uniquequeue.h
        stream/gammalut16.h
        stream/jpegutils.h
        stream/ccvt.h
        stream/ccvt_types.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen/stream
        COMPONENT Devel
    )

    install(FILES
        stream/encoder/encodermanager.h
        stream/encoder/encoderinterface.h
        stream/encoder/rawencoder.h
        stream/encoder/mjpegencoder.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen/stream/encoder
        COMPONENT Devel
    )

    install(FILES
        stream/recorder/recordermanager.h
        stream/recorder/recorderinterface.h
        stream/recorder/serrecorder.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen/stream/recorder
        COMPONENT Devel
    )

    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
        list(APPEND ${PROJECT_NAME}_SOURCES
            webcam/v4l2_colorspace.c
            webcam/v4l2_base.cpp
            webcam/v4l2_decode/v4l2_decode.cpp
            webcam/v4l2_decode/v4l2_builtin_decoder.cpp
        )

        install(FILES
            webcam/v4l2_decode/v4l2_decode.h
            webcam/v4l2_decode/v4l2_builtin_decoder.h
            webcam/v4l2_colorspace.h
            DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen
            COMPONENT Devel
        )

        include_directories(webcam)
    endif()
endif()

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    hydrogendriver.c
    hydrogendriverio.c
    hydrogendrivermain.cpp
    defaultdevice.cpp
    timer/timer.cpp
    timer/elapsedtimer.cpp
    thread/single_thread.cpp
    hydrogenccd.cpp
    hydrogenccdchip.cpp
    hydrogensensorinterface.cpp
    hydrogencorrelator.cpp
    hydrogendetector.cpp
    hydrogenspectrograph.cpp
    hydrogenreceiver.cpp
    hydrogentelescope.cpp
    hydrogenfilterwheel.cpp
    hydrogenfocuserinterface.cpp
    hydrogenweatherinterface.cpp
    hydrogenfocuser.cpp
    hydrogenrotator.cpp
    hydrogenusbdevice.cpp
    hydrogenguiderinterface.cpp
    hydrogenfilterinterface.cpp
    hydrogenrotatorinterface.cpp
    hydrogendome.cpp
    hydrogengps.cpp
    hydrogenweather.cpp
    hydrogendustcapinterface.cpp
    hydrogenlightboxinterface.cpp
    hydrogenlogger.cpp
    hydrogencontroller.cpp
    connection/connectioninterface.cpp
    connection/connectionserial.cpp
    connection/connectiontcp.cpp
    dsp/manager.cpp
    dsp/dspinterface.cpp
    dsp/transforms.cpp
    dsp/convolution.cpp
    pid/pid.cpp
    fitskeyword.cpp

    property/hydrogenproperties.cpp
    property/hydrogenproperty.cpp
    property/hydrogenpropertybasic.cpp
    property/hydrogenpropertyview.cpp
    property/hydrogenpropertytext.cpp
    property/hydrogenpropertynumber.cpp
    property/hydrogenpropertyswitch.cpp
    property/hydrogenpropertylight.cpp
    property/hydrogenpropertyblob.cpp

    # connection/ttybase.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    hydrogendriver.h
    pid/pid.hpp
    defaultdevice.h
    hydrogenccd.h
    hydrogenccdchip.h
    hydrogensensorinterface.h
    hydrogencorrelator.h
    hydrogendetector.h
    hydrogenspectrograph.h
    hydrogenreceiver.h
    hydrogenfilterwheel.h
    hydrogenfocuserinterface.h
    hydrogenweatherinterface.h
    hydrogenfocuser.h
    hydrogenrotator.h
    hydrogentelescope.h
    hydrogenguiderinterface.h
    hydrogenfilterinterface.h
    hydrogenrotatorinterface.h
    timer/timer.hpp
    timer/elapsedtimer.hpp
    thread/single_thread.hpp
    hydrogendome.h
    hydrogengps.h
    hydrogenlightboxinterface.h
    hydrogendustcapinterface.h
    hydrogenweather.h
    hydrogenlogger.h
    hydrogencontroller.h
    hydrogenusbdevice.h
    fitskeyword.h

    property/hydrogenproperties.h
    property/hydrogenproperty.h
    property/hydrogenpropertybasic.h
    property/hydrogenpropertyview.h
    property/hydrogenpropertytext.h
    property/hydrogenpropertynumber.h
    property/hydrogenpropertyswitch.h
    property/hydrogenpropertylight.h
    property/hydrogenpropertyblob.h

    property/hydrogenwidgetview.h
    property/hydrogenwidgettraits.h
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    property/hydrogenproperty_p.h
    property/hydrogenproperties_p.h
    property/hydrogenpropertyblob_p.h
    property/hydrogenpropertyswitch_p.h
    property/hydrogenpropertylight_p.h
    property/hydrogenpropertytext_p.h
    property/hydrogenpropertynumber_p.h
    property/hydrogenpropertybasic_p.h
    # TODO
)

# Build Object Library
add_library(${PROJECT_NAME}_OBJECT OBJECT)
set_property(TARGET ${PROJECT_NAME}_OBJECT PROPERTY POSITION_INDEPENDENT_CODE 1)

target_compile_definitions(${PROJECT_NAME}_OBJECT PRIVATE "-DHAVE_LIBNOVA")

if(WIN32)
target_link_libraries(${PROJECT_NAME}_OBJECT setupapi wsock32 ws2_32 shlwapi iphlpapi)
endif()

target_sources(${PROJECT_NAME}_OBJECT
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

install(FILES
    ${${PROJECT_NAME}_HEADERS}
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/libhydrogen
    COMPONENT Devel
)

install(FILES
    connection/connectioninterface.h
    connection/connectionserial.h
    connection/connectiontcp.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen/connection
    COMPONENT Devel
)

install(FILES
    dsp/manager.h
    dsp/dspinterface.h
    dsp/transforms.h
    dsp/convolution.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/libhydrogen/dsp
    COMPONENT Devel
)

    add_library(${PROJECT_NAME}static STATIC)

    target_link_libraries(${PROJECT_NAME}static ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
    target_link_libraries(${PROJECT_NAME}static ${CMAKE_THREAD_LIBS_INIT})
    target_include_directories(${PROJECT_NAME}static PUBLIC .)

    set_target_properties(${PROJECT_NAME}static PROPERTIES
        VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
        SOVERSION ${HYDROGEN_SOVERSION}
        OUTPUT_NAME ${PROJECT_NAME} # this same name like shared library - backwards compatibility
    )

    install(TARGETS ${PROJECT_NAME}static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    add_library(${PROJECT_NAME} SHARED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

    target_include_directories(${PROJECT_NAME} PUBLIC .)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${CMAKE_HYDROGEN_VERSION_STRING}
        SOVERSION ${HYDROGEN_SOVERSION}
        OUTPUT_NAME ${PROJECT_NAME}
    )

    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
