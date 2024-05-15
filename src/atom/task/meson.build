project('atom-task', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_task_sources = [
  'loop_task.cpp',
  'conditional_task.cpp',
  'task.cpp'
]

atom_task_headers = [
  'loop_task.hpp',
  'conditional_task.hpp',
  'task.hpp'
]

# 依赖
thread_dep = dependency('threads')

atom_task_deps = [thread_dep]

# 对象库
atom_task_object = static_library('atom_task_object',
  sources: atom_task_sources,
  dependencies: atom_task_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_task_lib = static_library('atom-task',
  sources: atom_task_object.extract_all_objects(),
  dependencies: atom_task_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_task_headers, subdir: 'atom-task')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_task_lib.set_version(atom_hydrogen_version_string)
atom_task_lib.set_soversion(atom_hydrogen_soversion)
atom_task_lib.set_output_name('atom-task')