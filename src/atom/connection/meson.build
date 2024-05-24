project('atom-connection', 'c', 'cpp',
  version : '1.0.0',
  license : 'GPL3'
)

# C++ standard
cpp_std = 'c++11'

# Sources
sources = [
  'fifoclient.cpp',
  'fifoserver.cpp',
  'sockethub.cpp',
  'tcpclient.cpp',
  'udp_server.cpp',
  'udpclient.cpp'
]

# Headers
headers = [
  'fifoclient.hpp',
  'fifoserver.hpp',
  'sockethub.hpp',
  'tcpclient.hpp',
  'udp_server.hpp',
  'udpclient.hpp'
]

if get_option('enable_libssh')
  sources += [
    'sshclient.cpp',
    'sshserver.cpp'
  ]
  headers += [
    'sshclient.hpp',
    'sshserver.hpp'
  ]
endif

# Dependencies
loguru_dep = dependency('loguru')
threads_dep = dependency('threads')

libs = [
  loguru_dep,
  threads_dep
]

if get_option('enable_ssh')
  libssh_dep = dependency('libssh')
  libs += [libssh_dep]
endif

# Build Object Library
atom_connection_obj = static_library('atom-connection-object',
  sources,
  cpp_args : ['-fPIC'],
  dependencies : libs
)

# Build Static Library
atom_connection = static_library('atom-connection',
  [],
  link_with : atom_connection_obj,
  dependencies : libs,
  include_directories : include_directories('.')
)

# Set version properties
atom_connection.soversion = '1'
atom_connection.version = '0.1'

# Install
install_headers(headers, subdir : 'atom-connection')
install_library(atom_connection, subdir : get_option('libdir'))

# Python binding
if get_option('atom_build_python')
  pybind11_dep = dependency('pybind11')
  py_module = import('python')
  pybind_module = py_module.extension_module('atom-connection-py',
    'pybind.cpp',
    link_with : atom_connection,
    dependencies : [pybind11_dep]
  )

  if host_machine.system() == 'windows'
    pybind_module.link_with : 'ws2_32'
  endif
endif