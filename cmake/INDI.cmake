################################# INDI ################################
IF(USE_PLUGIN_TELESCOPECONTROL AND NOT WIN32)

    SET(PREFER_SYSTEM_INDILIB 1 CACHE BOOL "Use system-provided INDI instead of the bundled version")

    # Attempt to use system-provided INDI library
    find_library(INDICLIENT_LIB indiclient)
    if(INDICLIENT_LIB AND PREFER_SYSTEM_INDILIB)
        MESSAGE(STATUS "Using system-provided indiclient at ${INDICLIENT_LIB}")
        add_library(indiclient UNKNOWN IMPORTED GLOBAL)
        set_target_properties(indiclient PROPERTIES IMPORTED_LOCATION "${INDICLIENT_LIB}")

    else()
        # Check for specific system symbols
        include(CheckSymbolExists)
        check_symbol_exists(mremap sys/mman.h HAVE_MREMAP)
        check_symbol_exists(timespec_get time.h HAVE_TIMESPEC_GET)
        check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)

        # Download bundled INDI library
        CPMAddPackage(
            NAME indiclient
            URL https://github.com/indilib/indi/archive/v2.1.0.zip
            URL_HASH SHA256=551d23f8ea68b37c9b6504b6e5e55d32319d7605f2a63d78cfc73c2d95cee8f2
            VERSION 2.1.0
            DOWNLOAD_ONLY YES
        )

        # Apply fixes to source files
        function(apply_fix input_file output_file pattern replacement)
            file(READ ${input_file} file_content)
            string(REGEX REPLACE "${pattern}" "${replacement}" file_content "${file_content}")
            file(WRITE ${output_file} "${file_content}")
            configure_file(${output_file} ${input_file} COPYONLY)
        endfunction()

        apply_fix(${indiclient_SOURCE_DIR}/libs/indicore/indidevapi.h
                  ${indiclient_SOURCE_DIR}/libs/indicore/indidevapi.h.new
                  "#include .lilxml.h."
                  "#include \"lilxml.h\"\n#include <stdarg.h>")

        apply_fix(${indiclient_SOURCE_DIR}/libs/indicore/libastro.h
                  ${indiclient_SOURCE_DIR}/libs/indicore/libastro.h.new
                  "#include <libnova/utility.h>"
                  "")

        apply_fix(${indiclient_SOURCE_DIR}/libs/indibase/inditelescope.h
                  ${indiclient_SOURCE_DIR}/inditelescope.h.new
                  "#include <libnova/julian_day.h>"
                  "struct ln_date;")

        apply_fix(${indiclient_SOURCE_DIR}/libs/indibase/indilogger.h
                  ${indiclient_SOURCE_DIR}/indilogger.h.new
                  "#include <sys/time.h>"
                  "#ifdef Q_OS_WIN\n#include <windows.h>\n#else\n#include <sys/time.h>\n#endif")

        # Set library version
        set(INDI_SOVERSION "2")
        set(CMAKE_INDI_VERSION_MAJOR 2)
        set(CMAKE_INDI_VERSION_MINOR 1)
        set(CMAKE_INDI_VERSION_RELEASE 0)
        set(CMAKE_INDI_VERSION_STRING "${CMAKE_INDI_VERSION_MAJOR}.${CMAKE_INDI_VERSION_MINOR}.${CMAKE_INDI_VERSION_RELEASE}")
        set(INDI_VERSION ${CMAKE_INDI_VERSION_STRING})
        set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/indi/")

        configure_file(${indiclient_SOURCE_DIR}/config.h.cmake ${indiclient_SOURCE_DIR}/config.h)
        configure_file(${indiclient_SOURCE_DIR}/libs/indicore/indiapi.h.in ${indiclient_SOURCE_DIR}/libs/indiapi.h)

        # Define source files for the library
        list(APPEND INDILIB_SOURCES
            ${indiclient_SOURCE_DIR}/libs/indicore/lilxml.cpp
            ${indiclient_SOURCE_DIR}/libs/indicore/base64.c
            ${indiclient_SOURCE_DIR}/libs/indicore/indidevapi.c
            ${indiclient_SOURCE_DIR}/libs/indicore/indicom.c
            ${indiclient_SOURCE_DIR}/libs/indicore/userio.c
            ${indiclient_SOURCE_DIR}/libs/indicore/indiuserio.c
            ${indiclient_SOURCE_DIR}/libs/indiabstractclient/abstractbaseclient.cpp
            ${indiclient_SOURCE_DIR}/libs/indiclient/baseclient.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/basedevice.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/indibase.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/indistandardproperty.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/parentdevice.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/watchdeviceproperty.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indiproperties.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertybasic.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertyblob.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indiproperty.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertylight.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertynumber.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertyswitch.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertytext.cpp
            ${indiclient_SOURCE_DIR}/libs/indidevice/property/indipropertyview.cpp
            ${indiclient_SOURCE_DIR}/libs/sockets/tcpsocket.cpp
        )

        if(WIN32)
            list(APPEND INDILIB_SOURCES ${indiclient_SOURCE_DIR}/libs/sockets/tcpsocket_win.cpp)
        else()
            list(APPEND INDILIB_SOURCES ${indiclient_SOURCE_DIR}/libs/sockets/tcpsocket_unix.cpp)
        endif()

        # Build the static library
        add_library(indiclient STATIC ${INDILIB_SOURCES})

        # Configure target properties
        target_compile_definitions(indiclient
            PUBLIC
                $<$<BOOL:${HAVE_TIMESPEC_GET}>:HAVE_TIMESPEC_GET>
                $<$<BOOL:${HAVE_CLOCK_GETTIME}>:HAVE_CLOCK_GETTIME>
        )

        target_include_directories(indiclient
            PRIVATE
                ${CMAKE_CURRENT_BINARY_DIR}/libindi
                ${indiclient_SOURCE_DIR}/libindi
                ${indiclient_SOURCE_DIR}/libs
            PUBLIC
                ${CMAKE_CURRENT_BINARY_DIR}
                ${indiclient_SOURCE_DIR}
                ${indiclient_SOURCE_DIR}/libs/sockets
                ${indiclient_SOURCE_DIR}/libs/indiabstractclient
                ${indiclient_SOURCE_DIR}/libs/indibase
                ${indiclient_SOURCE_DIR}/libs/indicore
                ${indiclient_SOURCE_DIR}/libs/indiclient
                ${indiclient_SOURCE_DIR}/libs/indidevice
                ${indiclient_SOURCE_DIR}/libs/indidevice/property
        )

        target_link_libraries(indiclient ${ZLIB_LIBRARIES})
    endif()

ENDIF()
