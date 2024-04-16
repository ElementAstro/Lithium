project('Atom', 'cpp')

# Define sources and headers
atom_sources = [
    'error/error_stack.cpp',
    'log/logger.cpp',
    'log/global_logger.cpp',
    'log/syslog.cpp'
]
atom_headers = [
    'error/error_code.hpp',
    'error/error_stack.hpp',
    'log/logger.hpp',
    'log/global_logger.hpp',
    'log/syslog.hpp'
]

# Private headers
atom_private_headers = []

# Define libraries
atom_libs = [
    'loguru',
    'cpp_httplib',
    'libzippp',
    'atom-async',
    'atom-task',
    'atom-io',
    'atom-driver',
    'atom-event',
    'atom-experiment',
    'atom-component',
    'atom-type',
    'atom-utils',
    'atom-search',
    'atom-web',
    'atom-system',
    'atom-server'
]

# Check if fmt library is available
has_fmt = find_library('fmt')

if not has_fmt.found()
    atom_libs += 'fmt'
endif

# Create object library
atom_obj = library('atom_obj', 
    atom_sources + atom_headers + atom_private_headers,
    cpp_std: c_std
)

# Link with necessary libraries
foreach lib: atom_libs
    atom_obj_link_libs += dependency(lib)
endforeach

atom_obj.link_with(atom_obj_link_libs)

# Create static library
atom_static = static_library('atom_static', atom_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_static.version = version
atom_static.soversion = soversion
atom_static.basename = 'Atom'

# Install target
install_targets(atom_static)
