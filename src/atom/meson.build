project('atom', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 检查是否支持 std::format
has_std_format = run_command('echo "#include <format>" | cpp -E -', check: true).returncode() == 0

# 版本信息
atom_version_major = 1
atom_version_minor = 0
atom_version_release = 0
atom_soversion = atom_version_major
atom_version_string = '@0@.@1@.@2@'.format(atom_version_major, atom_version_minor, atom_version_release)

# Python 支持
python_dep = dependency('python', required: false)
pybind11_dep = dependency('pybind11', required: false)

atom_build_python = python_dep.found() and pybind11_dep.found()

if atom_build_python
  message('Found Python and pybind11.')
endif

# 子目录
subdir('algorithm')
subdir('async')
subdir('components')
subdir('connection')
subdir('error')
subdir('io')
subdir('log')
subdir('search')
subdir('system')
subdir('task')
subdir('type')
subdir('utils')
subdir('web')

# 如果没有 std::format，使用 fmt 库
fmt_dep = dependency('fmt', required: not has_std_format)

# 依赖
loguru_dep = dependency('loguru')
cpp_httplib_dep = dependency('cpp_httplib')
libzippp_dep = dependency('libzippp')

atom_libs = [
  loguru_dep,
  cpp_httplib_dep,
  libzippp_dep,
  'atom-algorithm',
  'atom-async',
  'atom-task',
  'atom-io',
  'atom-component',
  'atom-type',
  'atom-utils',
  'atom-search',
  'atom-web',
  'atom-system'
]

# 源文件和头文件
atom_sources = [
  'log/logger.cpp',
  'log/global_logger.cpp',
  'log/syslog.cpp'
]

atom_headers = [
  'log/logger.hpp',
  'log/global_logger.hpp',
  'log/syslog.hpp'
]

# 对象库
atom_object = static_library('atom_object',
  sources: atom_sources,
  dependencies: atom_libs,
  include_directories: include_directories('.'),
  cpp_args: ['-DHAVE_LIBNOVA'],
  install: false
)

# 静态库
atom_static = static_library('atom',
  sources: atom_object.extract_all_objects(),
  dependencies: atom_libs,
  version: atom_version_string,
  soversion: atom_soversion,
  install: true
)

# 安装
install_headers(atom_headers, subdir: 'atom')