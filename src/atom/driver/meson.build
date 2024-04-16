project('atom-driver', 'cpp')

# Set minimum Meson version required
meson_version = '>=0.59.0'

# Define sources and headers
sources = [
    'fitskeyword.cpp',
    'device.cpp',
    'camera.cpp',
    'telescope.cpp',
    'focuser.cpp',
    'filterwheel.cpp',
    'guider.cpp',
    'solver.cpp'
]
headers = [
    'fitskeyword.h',
    'device.hpp',
    'camera.hpp',
    'telescope.hpp',
    'focuser.hpp',
    'filterwheel.hpp',
    'guider.hpp',
    'solver.hpp'
]

# Check if fmt library is available
has_fmt = find_library('fmt')

# Create object library
atom_driver_obj = library('atom_driver_obj', 
    sources + headers,
    cpp_std: c_std
)

# Link with necessary libraries
atom_driver_obj_link_libs = ['atom-component', 'atom-server']
atom_driver_obj.link_with(atom_driver_obj_link_libs)

if not has_fmt.found()
    atom_driver_obj_link_libs += ['fmt']
endif

if host_machine.system() == 'linux'
    atom_driver_obj_link_libs += ['uuid']
endif

atom_driver_obj.link_with(atom_driver_obj_link_libs)

# Create shared library
atom_driver_shared = shared_library('atom_driver_shared', atom_driver_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_driver_shared.version = version
atom_driver_shared.soversion = soversion
atom_driver_shared.basename = 'atom-driver'

# Install target
install_targets(atom_driver_shared)

# Create executable
shared_driver_test = executable('shared_driver_test', 'main.cpp', link_with : [atom_driver_shared])
