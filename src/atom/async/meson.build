project('Atom-Async', 'cpp')

# Define sources and headers
sources = [
    'lock.cpp',
    'timer.cpp'
]
headers = [
    'async.hpp',
    'async_impl.hpp',
    'lock.hpp',
    'queue.hpp',
    'queue.inl',
    'thread_wrapper.hpp',
    'timer.hpp',
    'trigger.hpp',
    'trigger_impl.hpp'
]

# Create object library
atom_async_obj = library('atom_async_obj', 
    sources + headers,
    cpp_std: c_std
)

# Link with necessary libraries
atom_async_obj_link_libs = ['loguru']
atom_async_obj.link_with(atom_async_obj_link_libs)

# Create static library
atom_async_static = static_library('atom_async_static', atom_async_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_async_static.version = version
atom_async_static.soversion = soversion
atom_async_static.basename = 'Atom-Async'

# Install target
install_targets(atom_async_static)

# Check if Python support is enabled
atom_build_python = get_option('atom_build_python')

if atom_build_python
    pybind11_module('atom_async_py' '_pybind.cpp' link_with : [atom_async_static])
endif
