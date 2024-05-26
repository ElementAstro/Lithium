project('atom-web', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 源文件和头文件
atom_web_sources = [
  'address.cpp',
  'downloader.cpp',
  'httpclient.cpp',
  'httplite.cpp',
  'httpparser.cpp',
  'utils.cpp',
  'time.cpp'
]

atom_web_headers = [
  'address.hpp',
  'downloader.hpp',
  'httpclient.hpp',
  'httplite.hpp',
  'httpparser.hpp',
  'utils.hpp',
  'time.hpp'
]

# 依赖
loguru_dep = dependency('loguru')
cpp_httplib_dep = dependency('cpp-httplib')
thread_dep = dependency('threads')

atom_web_deps = [loguru_dep, cpp_httplib_dep, thread_dep]

# Windows 特定库
win32_deps = []
if host_machine.system() == 'windows'
  win32_deps = ['wsock32', 'ws2_32']
endif

# 对象库
atom_web_object = static_library('atom_web_object',
  sources: atom_web_sources,
  dependencies: atom_web_deps + win32_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_web_lib = static_library('atom-web',
  sources: atom_web_object.extract_all_objects(),
  dependencies: atom_web_deps + win32_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_web_headers, subdir: 'atom-web')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_web_lib.set_version(atom_hydrogen_version_string)
atom_web_lib.set_soversion(atom_hydrogen_soversion)
atom_web_lib.set_output_name('atom-web')
