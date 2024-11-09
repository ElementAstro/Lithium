find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)
find_package(Readline REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
find_package(Python COMPONENTS Interpreter REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS} ${Python_INCLUDE_DIRS})