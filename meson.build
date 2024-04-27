project('Lithium', 'cpp')

lithium_src_dir = 'src'
lithium_module_dir = join_paths(lithium_src_dir, 'atom')
lithium_client_dir = join_paths(lithium_src_dir, 'client')
lithium_component_dir = join_paths(lithium_src_dir, 'addon')
lithium_task_dir = join_paths(lithium_src_dir, 'task')

libs_dir = 'libs'

lithium_deps = [
    dependency('oatpp-websocket'),
    dependency('oatpp-swagger'),
    dependency('oatpp-openssl'),
    dependency('oatpp-zlib'),
    dependency('loguru'),
    dependency('libzippp'),
    dependency('fmt'),
    dependency('cfitsio'),
    dependency('openssl'),
    dependency('sqlite3'),
    dependency('cpp_httplib'),
    dependency('backward'),
    dependency('tinyxml2'),
    dependency('pocketpy'),
]

cpp = meson.get_compiler('cpp')

lithium_server_src = files([
    join_paths(lithium_src_dir, 'app.cpp'),
])

lithium_server_library_src = files([
    join_paths(lithium_src_dir, 'device/server/ascom.cpp'),
    join_paths(lithium_src_dir, 'device/server/hydrogen.cpp'),
    join_paths(lithium_src_dir, 'device/server/hydrogen_driver.cpp'),
    join_paths(lithium_src_dir, 'device/server/connector.cpp'),
    join_paths(lithium_src_dir, 'device/manager.cpp'),
    join_paths(lithium_src_dir, 'device/utils/utils.cpp'),
    join_paths(lithium_component_dir, 'addons.cpp'),
    join_paths(lithium_component_dir, 'compiler.cpp'),
    join_paths(lithium_component_dir, 'finder.cpp'),
    join_paths(lithium_component_dir, 'loader.cpp'),
    join_paths(lithium_component_dir, 'manager.cpp'),
    join_paths(lithium_component_dir, 'sandbox.cpp'),
    join_paths(lithium_component_dir, 'sort.cpp'),
    join_paths(lithium_src_dir, 'config/configor.cpp'),
    join_paths(lithium_src_dir, 'debug/terminal.cpp'),
    join_paths(lithium_task_dir, 'manager.cpp'),
    join_paths(lithium_task_dir, 'generator.cpp'),
    join_paths(lithium_task_dir, 'container.cpp'),
    join_paths(lithium_task_dir, 'tick.cpp'),
    join_paths(lithium_task_dir, 'loader.cpp'),
    join_paths(lithium_task_dir, 'list.cpp'),
    join_paths(lithium_task_dir, 'pool.cpp'),
    join_paths(lithium_src_dir, 'LithiumApp.cpp'),
])

lithium_server_library = static_library(
    'lithium_server-library',
    lithium_server_library_src,
    dependencies: lithium_deps,
    include_directories: include_directories('.', 'libs', 'libs/oatpp', 'libs/oatpp-swagger', 'libs/oatpp-websocket'),
)

lithium_server = executable(
    'lithium_server',
    lithium_server_src,
    dependencies: lithium_deps,
    include_directories: include_directories('.', 'libs', 'libs/oatpp', 'libs/oatpp-swagger', 'libs/oatpp-websocket'),
    link_with: lithium_server_library,
    link_args: cpp.get_supported_link_arguments(),
)

configure_file(
    input: 'config.h.in',
    output: 'config.h',
)

