project('atom-type', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_type_sources = [
  'ini.cpp',
  'message.cpp'
]

atom_type_headers = [
  'args.hpp',
  'enum_flag.hpp',
  'enum_flag.inl',
  'flatset.hpp',
  'ini.inl',
  'ini.hpp',
  'json.hpp',
  'message.hpp',
  'pointer.hpp',
  'small_vector.hpp'
]

# 依赖
atom_utils_dep = dependency('atom-utils')

# 对象库
atom_type_object = static_library('atom_type_object',
  sources: atom_type_sources,
  dependencies: [atom_utils_dep],
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_type_lib = static_library('atom-type',
  sources: atom_type_object.extract_all_objects(),
  dependencies: [atom_utils_dep],
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_type_headers, subdir: 'atom-type')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_type_lib.set_version(atom_hydrogen_version_string)
atom_type_lib.set_soversion(atom_hydrogen_soversion)
atom_type_lib.set_output_name('atom-type')