project('Atom-Experiment', 'cpp')

# Define sources and headers
sources = [
    'platform.cpp',
    'string.cpp'
]
headers = [
    'any.hpp',
    'anyutils.hpp',
    'bind_first.hpp',
    'callable.hpp',
    'decorate.hpp',
    'flatmap.hpp',
    'func_traits.hpp',
    'invoke.hpp',
    'list.hpp',
    'memory.hpp',
    'noncopyable.hpp',
    'object.hpp',
    'optional.hpp',
    'platform.hpp',
    'ranges.hpp',
    'short_alloc.hpp',
    'stack_vector.hpp',
    'static_vector.hpp',
    'string.hpp',
    'to_string.hpp',
    'type_info.hpp'
]

# Create object library
atom_experiment_obj = library('atom_experiment', 
    sources + headers,
    cpp_std: c_std
)

# Create static library
atom_experiment_static = static_library('atom_experiment_static', atom_experiment_obj)

# Set version properties
version = '1.0.0' # You should define your version here
soversion = '1'

atom_experiment_static.version = version
atom_experiment_static.soversion = soversion
atom_experiment_static.basename = 'Atom-Experiment'

# Install target
install_targets(atom_experiment_static)
