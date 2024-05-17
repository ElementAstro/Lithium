project('loguru', 'cpp',
  version: '1.0.0',  # 替换为实际版本号
  license: 'GPL3',
  default_options: ['cpp_std=c++11']
)

# 选项
option('build_examples', type: 'boolean', value: false, description: 'Build the project examples')
option('build_tests', type: 'boolean', value: false, description: 'Build the tests')
option('install_library', type: 'boolean', value: true, description: 'Generate the install target(s)')
option('use_fmtlib', type: 'boolean', value: true, description: 'Use fmt library')

# 依赖项
threads_dep = dependency('threads')
fmt_dep = dependency('fmt', required: get_option('use_fmtlib'))
dlfcn_win32_dep = dependency('dlfcn-win32', required: host_machine.system() == 'windows')

# 包含目录
inc_dir = include_directories('.')

# 编译选项
warning_flags = [
  '-Wall', '-Wextra', '-Werror', '-pedantic',
  '-Weverything', '-Wno-c++98-compat', '-Wno-c++98-compat-pedantic',
  '/W4'
]

# 库
loguru_sources = [
  'loguru.cpp'
]

loguru_lib = library(
  'loguru',
  loguru_sources,
  include_directories: inc_dir,
  dependencies: [threads_dep, fmt_dep, dlfcn_win32_dep],
  version: '1.0.0',
  soversion: '1',
  install: get_option('install_library')
)

# 定义宏
loguru_defines = [
  'LOGURU_EXPORT',
  'LOGURU_DEBUG_LOGGING=1',
  'LOGURU_DEBUG_CHECKS=1',
  'LOGURU_SCOPE_TEXT_SIZE=128',
  'LOGURU_REDEFINE_ASSERT=0',
  'LOGURU_WITH_STREAMS=1',
  'LOGURU_REPLACE_GLOG=0',
  'LOGURU_USE_FMTLIB=1',
  'LOGURU_FMT_HEADER_ONLY=0',
  'LOGURU_WITH_FILEABS=0',
  'LOGURU_STACKTRACES=1'
]

loguru_lib.add_project_arguments(
  loguru_defines,
  language: 'cpp'
)

# 安装
if get_option('install_library')
  install_headers('loguru.hpp', subdir: 'loguru')
endif

# 测试
if get_option('build_tests')
  test_exe = executable(
    'loguru_test',
    'test.cpp',
    include_directories: inc_dir,
    dependencies: [loguru_lib]
  )
  test('Loguru Library Tests', test_exe)
endif

# 示例
if get_option('build_examples')
  example_exe = executable(
    'loguru_example',
    'example.cpp',
    include_directories: inc_dir,
    dependencies: [loguru_lib]
  )
  test('Loguru Library Example', example_exe)
endif
