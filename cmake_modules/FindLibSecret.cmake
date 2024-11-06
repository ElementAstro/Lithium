# - Try to find LIBSECRET-1
# Once done, this will define
#
#  LIBSECRET_FOUND - system has LIBSECRET
#  LIBSECRET_INCLUDE_DIRS - the LIBSECRET include directories
#  LIBSECRET_LIBRARIES - link these to use LIBSECRET

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LIBSECRET_PKGCONF LIBSECRET-1)

# Main include dir
find_path(LIBSECRET_INCLUDE_DIR
  NAMES LIBSECRET/secret.h
  PATHS ${LIBSECRET_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LIBSECRET_LIBRARY
  NAMES secret-1
  PATHS ${LIBSECRET_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LIBSECRET_PROCESS_INCLUDES LIBSECRET_INCLUDE_DIR)
set(LIBSECRET_PROCESS_LIBS LIBSECRET_LIBRARY)
libfind_process(LIBSECRET)
