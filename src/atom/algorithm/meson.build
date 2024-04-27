project('Atom-Algorithm', 'cpp')

# Define sources and headers
sources = [
    'algorithm.cpp',
    'base.cpp',
    'convolve.cpp',
    'fraction.cpp',
    'huffman.cpp',
    'math.cpp',
    'md5.cpp'
]
headers = [
    'algorithm.hpp',
    'algorithm.inl'
    'base.hpp',
    'convolve.hpp',
    'fraction.hpp',
    'hash.hpp',
    'huffman.hpp',
    'math.hpp',
    'md5.hpp'
]

# Create object library
atom_algorithm_obj = library('atom_algorithm_obj', 
    sources + headers,
    cpp_std: c_std
)

# Create static library
atom_algorithm_static = static_library('atom_algorithm_static', atom_algorithm_obj)

# Set version properties
version = '1.0.0'
soversion = '1'

atom_algorithm_static.version = version
atom_algorithm_static.soversion = soversion
atom_algorithm_static.basename = 'Atom-Algorithm'

# Install target
install_targets(atom_algorithm_static)

# Check if Python support is enabled
atom_build_python = get_option('atom_build_python')

if atom_build_python
    pybind11_module('atom_algorithm_py' 'pybind.cpp' link_with : [atom_algorithm_static])
endif
