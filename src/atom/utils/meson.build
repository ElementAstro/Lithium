project('atom-utils', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_utils_sources = [
  'aes.cpp',
  'random.cpp',
  'string.cpp',
  'stopwatcher.cpp',
  'time.cpp',
  'uuid.cpp',
  'xml.cpp'
]

atom_utils_headers = [
  'aes.hpp',
  'random.hpp',
  'refl.hpp',
  'string.hpp',
  'stopwatcher.hpp',
  'switch.hpp',
  'time.hpp',
  'uuid.hpp',
  'xml.hpp'
]

# 私有头文件（如果有）
atom_utils_private_headers = []

# 依赖
loguru_dep = dependency('loguru')
tinyxml2_dep = dependency('tinyxml2')
thread_dep = dependency('threads')

atom_utils_deps = [loguru_dep, tinyxml2_dep, thread_dep]

# 对象库
atom_utils_object = static_library('atom_utils_object',
  sources: atom_utils_sources,
  dependencies: atom_utils_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_utils_lib = static_library('atom-utils',
  sources: atom_utils_object.extract_all_objects(),
  dependencies: atom_utils_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_utils_headers, subdir: 'atom-utils')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_utils_lib.set_version(atom_hydrogen_version_string)
atom_utils_lib.set_soversion(atom_hydrogen_soversion)
atom_utils_lib.set_output_name('atom-utils')
