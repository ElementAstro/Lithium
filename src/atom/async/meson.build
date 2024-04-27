project('atom-async', 'cpp')

# Set sources
sources = files(
    'lock.cpp',
    'timer.cpp',
)

# Set headers
headers = files(
    'async.hpp',
    'async.inl',
    'lock.hpp',
    'pool.hpp',
    'queue.hpp',
    'queue.inl',
    'thread_wrapper.hpp',
    'timer.hpp',
    'trigger.hpp',
    'trigger.inl',
)

# Set libraries
libs = [
    'loguru',
]

# Build Object Library
atom_async_obj = static_library(
    'atom_async_obj',
    sources + headers,
    include_directories: '.',
    pic: true,
    dependencies: libs,
)

# Build Static Library
atom_async = static_library(
    'atom_async',
    sources: [],
    dependencies: [atom_async_obj],
    version: '1.0.0', # Replace with your version
    soversion: '1', # Replace with your SO version
)

# Install target
install_data(atom_async, install_dir: meson.install_root() / 'lib')

# Optional: Build Python bindings
if get_option('atom_build_python')
    pybind11_dep = dependency('pybind11')
    pybind11_add_module(
        'atom_async_py',
        'pybind.cpp',
        dependencies: [atom_async, pybind11_dep],
    )
endif