project('atom-component', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_component_sources = [
  'component.cpp',
  'dispatch.cpp'
]

atom_component_headers = [
  'component.hpp',
  'dispatch.hpp',
  'dispatch.inl',
  'types.hpp',
  'var.hpp',
  'var.inl'
]

# 依赖
loguru_dep = dependency('loguru')
atom_error_dep = dependency('atom-error', required: true)
atom_type_dep = dependency('atom-type', required: true)
atom_utils_dep = dependency('atom-utils', required: true)

atom_component_deps = [
  loguru_dep,
  atom_error_dep,
  atom_type_dep,
  atom_utils_dep
]

# 对象库
atom_component_object = static_library('atom_component_object',
  sources: atom_component_sources,
  dependencies: atom_component_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_component_lib = static_library('atom-component',
  sources: atom_component_object.extract_all_objects(),
  dependencies: atom_component_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_component_headers, subdir: 'atom-component')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_component_lib.set_version(atom_hydrogen_version_string)
atom_component_lib.set_soversion(atom_hydrogen_soversion)
atom_component_lib.set_output_name('atom-component')