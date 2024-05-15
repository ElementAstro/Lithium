project('atom-search', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_search_sources = [
  'search.cpp',
  'sqlite.cpp'
]

atom_search_headers = [
  'cache_impl.hpp',
  'cache.hpp',
  'search.hpp',
  'sqlite.hpp'
]

# 私有头文件（如果有）
atom_search_private_headers = []

# 依赖
loguru_dep = dependency('loguru')
thread_dep = dependency('threads')

atom_search_deps = [loguru_dep, thread_dep]

# 对象库
atom_search_object = static_library('atom_search_object',
  sources: atom_search_sources,
  dependencies: atom_search_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_search_lib = static_library('atom-search',
  sources: atom_search_object.extract_all_objects(),
  dependencies: atom_search_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_search_headers, subdir: 'atom-search')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_search_lib.set_version(atom_hydrogen_version_string)
atom_search_lib.set_soversion(atom_hydrogen_soversion)
atom_search_lib.set_output_name('atom-search')