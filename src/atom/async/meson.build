project('atom-async', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++20']
)

# 源文件和头文件
atom_async_sources = [
  'lock.cpp',
  'timer.cpp'
]

atom_async_headers = [
  'async.hpp',
  'async.inl',
  'lock.hpp',
  'pool.hpp',
  'queue.hpp',
  'queue.inl',
  'thread_wrapper.hpp',
  'timer.hpp',
  'trigger.hpp',
  'trigger.inl'
]

# 依赖
loguru_dep = dependency('loguru')

# 对象库
atom_async_object = static_library('atom_async_object',
  sources: atom_async_sources,
  dependencies: [loguru_dep],
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_async_lib = static_library('atom-async',
  sources: atom_async_object.extract_all_objects(),
  dependencies: [loguru_dep],
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_async_headers, subdir: 'atom-async')

# Python 绑定（如果需要）
pybind_enabled = get_option('build_python_binding')

if pybind_enabled
  pybind11_dep = dependency('pybind11', required: true)
  python_binding = import('python')
  atom_async_py = python_binding.extension_module('atom-async-py',
    sources: '_pybind.cpp',
    dependencies: [pybind11_dep],
    include_directories: include_directories('.')
  )
  atom_async_py.link_with(atom_async_lib)
endif

# 选项：是否构建 Python 绑定
option('build_python_binding', type: 'boolean', value: false, description: 'Build Python bindings using pybind11')