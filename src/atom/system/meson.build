project('atom-system', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL3',
  default_options: ['cpp_std=c++17']
)

# 版本信息
atom_system_version_major = 1
atom_system_version_minor = 0
atom_system_version_release = 0
atom_system_soversion = atom_system_version_major
atom_system_version_string = '@0@.@1@.@2@'.format(atom_system_version_major, atom_system_version_minor, atom_system_version_release)

# 源文件和头文件
atom_system_sources = [
  'command.cpp',
  'crash_quotes.cpp',
  'crash.cpp',
  'lregistry.cpp',
  'os.cpp',
  'pidwatcher.cpp',
  'platform.cpp',
  'process.cpp',
  'register.cpp',
  'storage.cpp',
  'system.cpp',
  'user.cpp',

  'module/cpu.cpp',
  'module/disk.cpp',
  'module/gpu.cpp',
  'module/memory.cpp',
  'module/battery.cpp',
  'module/wifi.cpp',
  'module/os.cpp'
]

atom_system_headers = [
  'command.hpp',
  'crash_quotes.hpp',
  'crash.hpp',
  'os.hpp',
  'pidwatcher.hpp',
  'platform.hpp',
  'process.hpp',
  'register.hpp',
  'storage.hpp',
  'system.hpp',
  'user.hpp',

  'module/cpu.hpp',
  'module/disk.hpp',
  'module/gpu.hpp',
  'module/memory.hpp',
  'module/battery.hpp',
  'module/wifi.hpp',
  'module/os.hpp'
]

# 依赖项
loguru_dep = dependency('loguru')
thread_dep = dependency('threads')

atom_system_deps = [loguru_dep, thread_dep]

# 链接 Windows 特定库
win32_deps = []
if host_machine.system() == 'windows'
  win32_deps = ['pdh', 'wlanapi']
endif

# 对象库
atom_system_object = static_library('atom_system_object',
  sources: atom_system_sources,
  dependencies: atom_system_deps + win32_deps,
  include_directories: include_directories('.'),
  install: false
)

# 静态库
atom_system_lib = static_library('atom-system',
  sources: atom_system_object.extract_all_objects(),
  dependencies: atom_system_deps + win32_deps,
  include_directories: include_directories('.'),
  install: true
)

# 安装头文件
install_headers(atom_system_headers, subdir: 'atom-system')

# 设置目标属性
atom_hydrogen_version_string = '1.0.0'
atom_hydrogen_soversion = '1'

atom_system_lib.set_version(atom_hydrogen_version_string)
atom_system_lib.set_soversion(atom_hydrogen_soversion)
atom_system_lib.set_output_name('atom-system')
