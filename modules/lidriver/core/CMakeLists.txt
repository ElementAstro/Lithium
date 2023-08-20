cmake_minimum_required(VERSION 3.13)
project(lithiumdriver C CXX)

find_package(USB1 REQUIRED)
find_package(JPEG REQUIRED)
include_directories(${USB1_INCLUDE_DIRS})

option(LITHIUM_BUILD_STATIC ON)

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
    lithiumcore
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
        DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium/stream
        COMPONENT Devel
    )

    install(FILES
        stream/encoder/encodermanager.h
        stream/encoder/encoderinterface.h
        stream/encoder/rawencoder.h
        stream/encoder/mjpegencoder.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium/stream/encoder
        COMPONENT Devel
    )

    install(FILES
        stream/recorder/recordermanager.h
        stream/recorder/recorderinterface.h
        stream/recorder/serrecorder.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium/stream/recorder
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
            DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium
            COMPONENT Devel
        )

        include_directories(webcam)
    endif()
endif()

# Sources
list(APPEND ${PROJECT_NAME}_SOURCES
    lithiumdriver.c
    lithiumdriverio.c
    lithiumdrivermain.c
    defaultdevice.cpp
    timer/timer.cpp
    timer/elapsedtimer.cpp
    thread/single_thread.cpp
    lithiumccd.cpp
    lithiumccdchip.cpp
    lithiumsensorinterface.cpp
    lithiumcorrelator.cpp
    lithiumdetector.cpp
    lithiumspectrograph.cpp
    lithiumreceiver.cpp
    lithiumtelescope.cpp
    lithiumfilterwheel.cpp
    lithiumfocuserinterface.cpp
    lithiumweatherinterface.cpp
    lithiumfocuser.cpp
    lithiumrotator.cpp
    lithiumusbdevice.cpp
    lithiumguiderinterface.cpp
    lithiumfilterinterface.cpp
    lithiumrotatorinterface.cpp
    lithiumdome.cpp
    lithiumgps.cpp
    lithiumweather.cpp
    lithiumdustcapinterface.cpp
    lithiumlightboxinterface.cpp
    lithiumlogger.cpp
    lithiumcontroller.cpp
    connection/connectioninterface.cpp
    connection/connectionserial.cpp
    connection/connectiontcp.cpp
    dsp/manager.cpp
    dsp/dspinterface.cpp
    dsp/transforms.cpp
    dsp/convolution.cpp
    pid/pid.cpp
    fitskeyword.cpp

    property/lithiumproperties.cpp
    property/lithiumproperty.cpp
    property/lithiumpropertybasic.cpp
    property/lithiumpropertyview.cpp
    property/lithiumpropertytext.cpp
    property/lithiumpropertynumber.cpp
    property/lithiumpropertyswitch.cpp
    property/lithiumpropertylight.cpp
    property/lithiumpropertyblob.cpp

    # connection/ttybase.cpp
)

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    lithiumdriver.h
    pid/pid.hpp
    defaultdevice.h
    lithiumccd.h
    lithiumccdchip.h
    lithiumsensorinterface.h
    lithiumcorrelator.h
    lithiumdetector.h
    lithiumspectrograph.h
    lithiumreceiver.h
    lithiumfilterwheel.h
    lithiumfocuserinterface.h
    lithiumweatherinterface.h
    lithiumfocuser.h
    lithiumrotator.h
    lithiumtelescope.h
    lithiumguiderinterface.h
    lithiumfilterinterface.h
    lithiumrotatorinterface.h
    timer/timer.hpp
    timer/elapsedtimer.hpp
    thread/single_thread.hpp
    lithiumdome.h
    lithiumgps.h
    lithiumlightboxinterface.h
    lithiumdustcapinterface.h
    lithiumweather.h
    lithiumlogger.h
    lithiumcontroller.h
    lithiumusbdevice.h
    fitskeyword.h

    property/lithiumproperties.h
    property/lithiumproperty.h
    property/lithiumpropertybasic.h
    property/lithiumpropertyview.h
    property/lithiumpropertytext.h
    property/lithiumpropertynumber.h
    property/lithiumpropertyswitch.h
    property/lithiumpropertylight.h
    property/lithiumpropertyblob.h

    property/lithiumwidgetview.h
    property/lithiumwidgettraits.h
)

# Private Headers
list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
    property/lithiumproperty_p.h
    property/lithiumproperties_p.h
    property/lithiumpropertyblob_p.h
    property/lithiumpropertyswitch_p.h
    property/lithiumpropertylight_p.h
    property/lithiumpropertytext_p.h
    property/lithiumpropertynumber_p.h
    property/lithiumpropertybasic_p.h
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
    ${INCLUDE_INSTALL_DIR}/liblithium
    COMPONENT Devel
)

install(FILES
    connection/connectioninterface.h
    connection/connectionserial.h
    connection/connectiontcp.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium/connection
    COMPONENT Devel
)

install(FILES
    dsp/manager.h
    dsp/dspinterface.h
    dsp/transforms.h
    dsp/convolution.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/liblithium/dsp
    COMPONENT Devel
)

# Build Static Library
if(LITHIUM_BUILD_STATIC)
    add_library(${PROJECT_NAME}static STATIC)

    target_link_libraries(${PROJECT_NAME}static ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})
    target_include_directories(${PROJECT_NAME}static PUBLIC .)

    set_target_properties(${PROJECT_NAME}static PROPERTIES
        VERSION ${CMAKE_LITHIUM_VERSION_STRING}
        SOVERSION ${LITHIUM_SOVERSION}
        OUTPUT_NAME ${PROJECT_NAME} # this same name like shared library - backwards compatibility
    )

    install(TARGETS ${PROJECT_NAME}static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Build Shared Library
if(LITHIUM_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_OBJECT ${${PROJECT_NAME}_LIBS})

    target_include_directories(${PROJECT_NAME} PUBLIC .)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${CMAKE_LITHIUM_VERSION_STRING}
        SOVERSION ${LITHIUM_SOVERSION}
        OUTPUT_NAME ${PROJECT_NAME}
    )

    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
