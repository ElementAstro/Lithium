project('Atom-Connection', 'cpp')

# Define sources and headers
sources = [
    'fifoclient.cpp',
    'fifoserver.cpp',
    'sockethub.cpp',
    'udp_server.cpp'
]
headers = [
    'fifoclient.hpp',
    'fifoserver.hpp',
    'shared_memory.hpp',
    'sockethub.hpp',
    'udp_server.hpp'
]

# Check if LibSSH is enabled
enable_ssh = get_option('ENABLE_SSH')
enable_libssh = get_option('ENABLE_LIBSSH')

if enable_ssh and enable_libssh
    sources += 'sshclient.cpp'
    headers += 'sshclient.hpp'
endif

# Create object library
atom_connection_obj = library('atom_connection_obj', 
    sources + headers,
    cpp_std: c_std
)

# Link with necessary libraries
atom_connection_obj_link_libs = ['loguru']
atom_connection_obj.link_with(atom_connection_obj_link_libs)

# Create static library
atom_connection_static = static_library('atom_connection_static', atom_connection_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_connection_static.version = version
atom_connection_static.soversion = soversion
atom_connection_static.basename = 'Atom-Connection'

# Install target
install_targets(atom_connection_static)

# Check if Python support is enabled
atom_build_python = get_option('ATOM_BUILD_PYTHON')

if atom_build_python
    pybind11_module('atom_connection_py' 'pybind.cpp' link_with : [atom_connection_static])
    if meson.get_build_option('WIN32')
        pybind11_module_link_with('atom_connection_py', ['ws2_32'])
    endif
endif
