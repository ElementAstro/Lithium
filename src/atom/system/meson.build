project('atom-system', 'cpp')

# Sources
atom_system_sources = [
    'command.cpp',
    'crash_quotes.cpp',
    'crash.cpp',
    'pidwatcher.cpp',
    'process.cpp',
    'register.cpp',
    'storage.cpp',
    'system.cpp',
    'user.cpp',

    'module/cpu.cpp',
    'module/disk.cpp',
    'module/memory.cpp',
    'module/battery.cpp',
    'module/wifi.cpp',
    'module/os.cpp',
]

# Headers
atom_system_headers = [
    'command.hpp',
    'crash_quotes.hpp',
    'crash.hpp',
    'pidwatcher.hpp',
    'process.hpp',
    'register.hpp',
    'storage.hpp',
    'system.hpp',
    'user.hpp',

    'module/cpu.hpp',
    'module/disk.hpp',
    'module/memory.hpp',
    'module/battery.hpp',
]

# Build Object Library
atom_system_object = library(
    'atom-system-object',
    atom_system_sources,
    include_directories: include_directories('.')
)

# Link libraries
atom_system_object_deps = [
    dependency('loguru')
]

# Build Static Library
atom_system_static = library(
    'atom-system',
    atom_system_sources,
    include_directories: include_directories('.'),
    dependencies: atom_system_object_deps,
    version: '1.0',
    soversion: '1'
)

# Install
install_targets(atom_system_static)
