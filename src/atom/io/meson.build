project('atom-io', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_io_sources = [
  'compress.cpp',
  'file.cpp',
  'io.cpp'
]

atom_io_headers = [
  'compress.hpp',
  'file.hpp',
  'glob.hpp',
  'io.hpp'
]

# 私有头文件（如果有）
atom_io_private_headers = []

# 依赖
loguru_dep = dependency('loguru')
libzippp_dep = dependency('libzippp')
thread_dep = dependency('threads')

atom_io_deps = [loguru_dep, libzippp_dep, thread_dep]

# 对象库
atom_io_object = static_library('atom_io_object',
  sources: atom_io_sources,
  dependencies: atom_io_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_io_lib = static_library('atom-io',
  sources: atom_io_object.extract_all_objects(),
  dependencies: atom_io_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_io_headers, subdir: 'atom-io')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_io_lib.set_version(atom_hydrogen_version_string)
atom_io_lib.set_soversion(atom_hydrogen_soversion)
atom_io_lib.set_output_name('atom-io')
