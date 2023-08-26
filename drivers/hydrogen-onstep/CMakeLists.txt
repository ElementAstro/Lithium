# ########## LX200 Generic ###########
add_executable(hydrogen_lx200generic
    lx200driver.cpp
    lx200autostar.cpp
    lx200_16.cpp
    lx200gps.cpp
    lx200generic.cpp
    lx200telescope.cpp
    lx200classic.cpp
    lx200gemini.cpp
    lx200zeq25.cpp
    lx200am5.cpp
    lx200gotonova.cpp
    lx200pulsar2.cpp
    lx200apdriver.cpp
    lx200ap_v2.cpp
    lx200fs2.cpp
    lx200ss2000pc.cpp
    lx200_OnStep.cpp
    lx200_OpenAstroTech.cpp
    lx200_pegasus_nyx101.cpp
    lx200_10micron.cpp
    ioptronHC8406.cpp
    eq500x.cpp)

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    target_compile_definitions(hydrogen_lx200generic PRIVATE -D_XOPEN_SOURCE=700)
ELSE()
    target_compile_definitions(hydrogen_lx200generic PRIVATE -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=200809L)
ENDIF()

target_link_libraries(hydrogen_lx200generic lithiumcore lithiumdevice dsp eventloop fpack lithiumdriver_OBJECT)

install(TARGETS hydrogen_lx200generic RUNTIME DESTINATION bin)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_lx200generic_symlink.cmake
    "exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200classic)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200autostar)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200_16)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200gps)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200ap_v2)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200gemini)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200zeq25)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200am5)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200gotonova)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200pulsar2)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200fs2)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200ss2000pc)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200_OnStep)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200_OpenAstroTech)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200_pegasus_nyx101)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_lx200_10micron)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_ioptronHC8406)\n
exec_program(\"${CMAKE_COMMAND}\" ARGS -E create_symlink hydrogen_lx200generic \$ENV{DESTDIR}${BIN_INSTALL_DIR}/hydrogen_eq500x_telescope)\n
")
set_target_properties(hydrogen_lx200generic PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_lx200generic_symlink.cmake)
