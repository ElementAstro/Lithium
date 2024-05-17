project('atom-algorithm', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++20']
)

# 源文件和头文件
atom_algorithm_sources = [
  'algorithm.cpp',
  'base.cpp',
  'convolve.cpp',
  'fraction.cpp',
  'huffman.cpp',
  'math.cpp',
  'md5.cpp',
  'mhash.cpp',
  'pid.cpp'
]

atom_algorithm_headers = [
  'algorithm.hpp',
  'algorithm.inl',
  'base.hpp',
  'convolve.hpp',
  'fraction.hpp',
  'hash.hpp',
  'huffman.hpp',
  'math.hpp',
  'md5.hpp',
  'mhash.hpp',
  'pid.hpp'
]

# 对象库
atom_algorithm_object = static_library('atom_algorithm_object',
  sources: atom_algorithm_sources,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_algorithm_lib = static_library('atom-algorithm',
  sources: atom_algorithm_object.extract_all_objects(),
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_algorithm_headers, subdir: 'atom-algorithm')

# Python 绑定（如果需要）
pybind_enabled = get_option('build_python_binding')

if pybind_enabled
  pybind11_dep = dependency('pybind11', required: true)
  python_binding = import('python')
  atom_algorithm_py = python_binding.extension_module('atom-algorithm-py',
    sources: '_pybind.cpp',
    dependencies: [pybind11_dep],
    include_directories: include_directories('.')
  )
  atom_algorithm_py.link_with(atom_algorithm_lib)
endif

# 选项：是否构建 Python 绑定
option('build_python_binding', type: 'boolean', value: false, description: 'Build Python bindings using pybind11')
