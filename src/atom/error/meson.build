project('atom-error', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_error_sources = [
  'error_stack.cpp',
  'exception.cpp',
  'stacktrace.cpp'
]

atom_error_headers = [
  'error_code.hpp',
  'error_stack.hpp',
  'stacktrace.hpp'
]

# 依赖
loguru_dep = dependency('loguru')
dl_dep = cc.find_library('dl', required: false)

atom_error_deps = [loguru_dep]
if dl_dep.found()
  atom_error_deps += [dl_dep]
endif

# 对象库
atom_error_object = static_library('atom_error_object',
  sources: atom_error_sources,
  dependencies: atom_error_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_error_lib = static_library('atom-error',
  sources: atom_error_object.extract_all_objects(),
  dependencies: atom_error_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_error_headers, subdir: 'atom-error')

# 设置目标属性
atom_error_lib.set_output_name('atom-error')