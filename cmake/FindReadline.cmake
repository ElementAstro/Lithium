# - Try to find the Readline library
# Once done, this will define
#  Readline_FOUND - System has Readline
#  Readline_INCLUDE_DIRS - The Readline include directories
#  Readline_LIBRARIES - The libraries needed to use Readline

find_path(Readline_INCLUDE_DIR
    NAMES readline/readline.h
    PATHS /usr/include /usr/local/include
)

find_library(Readline_LIBRARY
    NAMES readline
    PATHS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Readline DEFAULT_MSG
    Readline_LIBRARY Readline_INCLUDE_DIR
)

if(Readline_FOUND)
    set(Readline_LIBRARIES ${Readline_LIBRARY})
    set(Readline_INCLUDE_DIRS ${Readline_INCLUDE_DIR})
endif()

mark_as_advanced(Readline_INCLUDE_DIR Readline_LIBRARY)
