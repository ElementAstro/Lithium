project('atom-task', 'cpp')

# Sources
atom_task_sources = [
    'loop_task.cpp',
    'conditional_task.cpp',
    'task.cpp',
]

# Headers
atom_task_headers = [
    'loop_task.hpp',
    'conditional_task.hpp',
    'task.hpp',
]

# Build Object Library
atom_task_object = library(
    'atom-task-object',
    atom_task_sources,
    include_directories: include_directories('.')
)

# Build Static Library
atom_task_static = library(
    'atom-task',
    atom_task_sources,
    include_directories: include_directories('.'),
    dependencies: atom_task_object,
    version: '1.0',
    soversion: '1'
)

# Install
install_targets(atom_task_static)
