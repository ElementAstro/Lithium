project('Atom-Utils', 'cpp')

# Define sources and headers
sources = [
    'aes.cpp',
    'env.cpp',
    'hash_util.cpp',
    'random.cpp',
    'string.cpp',
    'stopwatcher.cpp',
    'time.cpp',
    'uuid.cpp',
    'xml.cpp'
]
headers = [
    'aes.hpp',
    'env.hpp',
    'hash_util.hpp',
    'random.hpp',
    'refl.hpp',
    'string.hpp',
    'stopwatcher.hpp',
    'switch.hpp',
    'time.hpp',
    'uuid.hpp',
    'xml.hpp'
]

# Private headers
private_headers = [
]

# Create object library
atom_utils_obj = library('atom_utils', 
    sources + headers + private_headers,
    cpp_std: c_std
)

# Link to other libraries if necessary
atom_utils_obj_link_libs = []
foreach lib : ['loguru', 'tinyxml2']
    atom_utils_obj_link_libs += dependency(lib)
endforeach

atom_utils_obj.link_with(atom_utils_obj_link_libs)

# Create static library
atom_utils_static = static_library('atom_utils_static', atom_utils_obj)

# Set version properties
version = '1.0.0'
soversion = '1'

atom_utils_static.version = version
atom_utils_static.soversion = soversion
atom_utils_static.basename = 'Atom-Utils'

# Install target
install_targets(atom_utils_static)
