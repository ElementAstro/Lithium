project('Atom-Type', 'cpp')

# Define sources and headers
sources = [
    'args.cpp',
    'ini.cpp',
    'message.cpp'
]
headers = [
    'abi.hpp',
    'args.hpp',
    'enum_flag.hpp',
    'enum_flag.inl',
    'flatset.hpp',
    'ini_impl.hpp',
    'ini.hpp',
    'json.hpp',
    'message.hpp',
    'pointer.hpp',
    'small_vector.hpp'
]

# Create object library
atom_type_obj = library('atom_type', 
    sources + headers,
    cpp_std: c_std
)

# Create static library
atom_type_static = static_library('atom_type_static', atom_type_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_type_static.version = version
atom_type_static.soversion = soversion
atom_type_static.basename = 'Atom-Type'

# Link to other libraries if necessary
atom_type_static_link_libs = []
foreach lib : ['atom-utils']
    atom_type_static_link_libs += dependency(lib)
endforeach

atom_type_static_link_libs = []
foreach lib : ['atom-utils']
    atom_type_static_link_libs += dependency(lib)
endforeach

atom_type_static.link_with(atom_type_static_link_libs)

# Install target
install_targets(atom_type_static)
