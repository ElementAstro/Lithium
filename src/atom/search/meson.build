project('atom-search', 'cpp')

# Sources
atom_search_sources = [
    'search.cpp',
    'sqlite.cpp',
]

# Headers
atom_search_headers = [
    'cache_impl.hpp',
    'cache.hpp',
    'search.hpp',
    'sqlite.hpp',
]

# Build Object Library
atom_search_object = library(
    'atom-search-object',
    atom_search_sources,
    include_directories: include_directories('.')
)

# Link libraries
atom_search_object_deps = dependency('loguru')

# Build Static Library
atom_search_static = library(
    'atom-search',
    atom_search_sources,
    include_directories: include_directories('.'),
    dependencies: [atom_search_object_deps],
    version: '1.0',
    soversion: '1'
)

# Install
install_targets(atom_search_static)
