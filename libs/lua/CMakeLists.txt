cmake_minimum_required(VERSION 3.6)

GET_FILENAME_COMPONENT(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})
#message(${PROJECT_NAME})
message("-- Build Lua version : 5.4.4")

set(srcFiles lapi.c lauxlib.c lbaselib.c lcode.c lcorolib.c lctype.c ldblib.c ldebug.c ldo.c
    ldump.c lfunc.c lgc.c linit.c liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c
    loslib.c lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c ltm.c lundump.c
    lutf8lib.c lvm.c lzio.c)
INCLUDE_DIRECTORIES(.)

# remove the lua.c/luac.c from teh all file lists
SET(LIB_SRC ${ALL_SRC})
LIST(REMOVE_ITEM LIB_SRC lua.c luac.c)

add_library(lua
    ${srcFiles})