project('atom-io', 'cpp')

# Sources
atom_io_sources = [
    'compress.cpp',
    'file.cpp',
    'io.cpp',
]

# Headers
atom_io_headers = [
    'compress.hpp',
    'file.hpp',
    'glob.hpp',
    'io.hpp',
]

# Build Object Library
atom_io_object = library(
    'atom-io-object',
    atom_io_sources,
    include_directories: include_directories('.')
)

# Link libraries
atom_io_object_deps = [
    dependency('loguru'),
    dependency('libzippp')
]

# Build Static Library
atom_io_static = library(
    'atom-io',
    atom_io_sources,
    include_directories: include_directories('.'),
    dependencies: atom_io_object_deps,
    version: '1.0',
    soversion: '1'
)

# Install
install_targets(atom_io_static)
