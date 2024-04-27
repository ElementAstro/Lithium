project('Atom-Web', 'cpp')

# Define sources and headers
sources = [
    'address.cpp',
    'downloader.cpp',
    'httpclient.cpp',
    'httplite.cpp',
    'utils.cpp',
    'time.cpp'
]
headers = [
    'address.hpp',
    'downloader.hpp',
    'httpclient.hpp',
    'httplite.hpp',
    'utils.hpp',
    'time.hpp'
]

# Create object library
atom_web_obj = library('atom_web', 
    sources + headers,
    cpp_std: c_std
)

# Link to other libraries if necessary
atom_web_obj_link_libs = []
foreach lib : ['loguru', 'cpp_httplib']
    atom_web_obj_link_libs += dependency(lib)
endforeach

atom_web_obj.link_with(atom_web_obj_link_libs)

# Create static library
atom_web_static = static_library('atom_web_static', atom_web_obj)

# Set version properties
version = '1.0.0'
soversion = '1'

atom_web_static.version = version
atom_web_static.soversion = soversion
atom_web_static.basename = 'Atom-Web'

# Install target
install_targets(atom_web_static)
