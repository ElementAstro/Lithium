project('Lithium', 'c', 'cpp', version: '1.0.0')

# Project directories
src_dir = meson.source_root() / 'src'
module_dir = src_dir / 'atom'
client_dir = src_dir / 'client'
component_dir = src_dir / 'addon'
task_dir = src_dir / 'task'

# Compiler options
add_project_arguments('-Wall', '-Wextra', '-Wpedantic', language: 'cpp')

# Include directories
inc_dirs = include_directories([
  'libs',
  'driverlibs',
  src_dir,
  module_dir,
  'libs/oatpp',
  'libs/oatpp-swagger',
  'libs/oatpp-websocket',
  meson.current_source_dir / 'modules'
])

# Dependencies
openssl_dep = dependency('openssl', required: true)
cfitsio_dep = dependency('cfitsio', required: true)
zlib_dep = dependency('zlib', required: true)
sqlite3_dep = dependency('sqlite3', required: true)
fmt_dep = dependency('fmt', required: true)

# Optional dependencies
loguru_lib = static_library('loguru', 'path/to/loguru.cpp') # Replace with actual path to loguru

# Subdirectories
subdir('libs')
subdir('tools')
subdir('src/atom')
subdir('modules')
subdir('driver')
subdir('src/carbon')
subdir('src/config')
subdir('src/server')

# Configuration options
enable_async = get_option('enable_async')
enable_debug = get_option('enable_debug')

config_h = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: {
    'ENABLE_ASYNC_FLAG': enable_async ? 1 : 0,
    'ENABLE_DEBUG_FLAG': enable_debug ? 1 : 0,
  }
)

# Source files
component_module = [
  component_dir / 'addons.cpp',
  component_dir / 'compiler.cpp',
  component_dir / 'loader.cpp',
  component_dir / 'manager.cpp',
  component_dir / 'sandbox.cpp',
  component_dir / 'sort.cpp'
]

config_module = [src_dir / 'config/configor.cpp']
debug_module = [src_dir / 'debug/terminal.cpp']
script_module = [
  src_dir / 'script/manager.cpp',
  src_dir / 'script/custom/sys.cpp',
  src_dir / 'script/custom/config.cpp',
  src_dir / 'script/sheller.cpp',
  src_dir / 'script/carbon.cpp'
]

task_module = [
  task_dir / 'manager.cpp',
  task_dir / 'generator.cpp',
  task_dir / 'container.cpp',
  task_dir / 'tick.cpp',
  task_dir / 'loader.cpp',
  task_dir / 'list.cpp',
  task_dir / 'pool.cpp'
]

lithium_module = src_dir / 'LithiumApp.cpp'

# Library
lithium_server_lib = static_library('lithium_server-library', [
  component_module,
  config_module,
  debug_module,
  task_module,
  script_module,
  lithium_module
], include_directories: inc_dirs, dependencies: [loguru_lib])

# Executable
lithium_server = executable('lithium_server', src_dir / 'App.cpp', 
  include_directories: inc_dirs,
  link_with: [
    lithium_server_lib, 
    # Add other libraries here
  ],
  dependencies: [
    openssl_dep,
    cfitsio_dep,
    zlib_dep,
    sqlite3_dep,
    fmt_dep
  ]
)

# Platform-specific settings
if host_machine.system() == 'windows'
  lithium_server.link_with('pdh', 'iphlpapi', 'winmm', 'crypt32', 'wsock32', 'ws2_32')
  dlfcn_dep = dependency('dlfcn-win32', required: true)
  lithium_server.link_with(dlfcn_dep)
elif host_machine.system() == 'linux'
  lithium_server.link_with('dl')
elif host_machine.system() == 'darwin'
  lithium_server.link_with('dl')
else
  error('Unsupported platform')
endif

# Install
install_data('config.h', install_dir: get_option('includedir'))
install_data(lithium_server, install_dir: get_option('bindir'))