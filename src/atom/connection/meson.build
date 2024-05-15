project('atom-connection', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 选项：是否启用 LibSSH 支持
option('enable_ssh', type: 'boolean', value: false, description: 'Enable SSH support with libssh')

# 依赖
loguru_dep = dependency('loguru')
ssh_dep = dependency('libssh', required: get_option('enable_ssh'))

# 源文件和头文件
atom_connection_sources = [
  'fifoclient.cpp',
  'fifoserver.cpp',
  'sockethub.cpp',
  'udp_server.cpp'
]

atom_connection_headers = [
  'fifoclient.hpp',
  'fifoserver.hpp',
  'shared_memory.hpp',
  'sockethub.hpp',
  'udp_server.hpp'
]

if get_option('enable_ssh')
  atom_connection_sources += ['sshclient.cpp']
  atom_connection_headers += ['sshclient.hpp']
endif

# 依赖项列表
atom_connection_deps = [loguru_dep]
if ssh_dep.found()
  atom_connection_deps += [ssh_dep]
endif

# 对象库
atom_connection_object = static_library('atom_connection_object',
  sources: atom_connection_sources,
  dependencies: atom_connection_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_connection_lib = static_library('atom-connection',
  sources: atom_connection_object.extract_all_objects(),
  dependencies: atom_connection_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_connection_headers, subdir: 'atom-connection')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_connection_lib.set_version(atom_hydrogen_version_string)
atom_connection_lib.set_soversion(atom_hydrogen_soversion)
atom_connection_lib.set_output_name('atom-connection')

# Python 绑定（如果需要）
pybind_enabled = get_option('build_python_binding')

if pybind_enabled
  pybind11_dep = dependency('pybind11', required: true)
  python_binding = import('python')
  atom_connection_py = python_binding.extension_module('atom-connection-py',
    sources: '_pybind.cpp',
    dependencies: [pybind11_dep] + atom_connection_deps,
    include_directories: include_directories('.')
  )
  if host_machine.system() == 'windows'
    atom_connection_py.link_with(atom_connection_lib)
    atom_connection_py.link_with('ws2_32')
  endif
endif

# 选项：是否构建 Python 绑定
option('build_python_binding', type: 'boolean', value: false, description: 'Build Python bindings using pybind11')