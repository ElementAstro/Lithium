import pytest
from tools.cmake_generator import generate_cmake, ProjectConfig


def test_generate_basic_cmake():
    config = ProjectConfig(project_name="TestProject")
    cmake_content = generate_cmake(config)
    assert "project(TestProject VERSION 1.0)" in cmake_content
    assert "set(CMAKE_CXX_STANDARD 11)" in cmake_content
    assert "add_executable(TestProject ${SOURCES})" in cmake_content


def test_include_directories():
    config = ProjectConfig(project_name="TestProject",
                           include_dirs=["include", "src"])
    cmake_content = generate_cmake(config)
    assert 'include_directories("include")' in cmake_content
    assert 'include_directories("src")' in cmake_content


def test_compiler_flags():
    config = ProjectConfig(project_name="TestProject",
                           compiler_flags=["-O3", "-Wall"])
    cmake_content = generate_cmake(config)
    assert "add_compile_options(-O3 -Wall)" in cmake_content


def test_linker_flags():
    config = ProjectConfig(project_name="TestProject",
                           linker_flags=["-lpthread"])
    cmake_content = generate_cmake(config)
    assert "add_link_options(-lpthread)" in cmake_content


def test_dependencies():
    config = ProjectConfig(project_name="TestProject",
                           dependencies=["Boost", "OpenCV"])
    cmake_content = generate_cmake(config)
    assert "find_package(Boost REQUIRED)" in cmake_content
    assert "find_package(OpenCV REQUIRED)" in cmake_content


def test_static_library():
    config = ProjectConfig(project_name="TestProject",
                           executable=False, static_library=True)
    cmake_content = generate_cmake(config)
    assert "add_library(TestProject STATIC ${SOURCES})" in cmake_content


def test_shared_library():
    config = ProjectConfig(project_name="TestProject",
                           executable=False, shared_library=True)
    cmake_content = generate_cmake(config)
    assert "add_library(TestProject SHARED ${SOURCES})" in cmake_content


def test_enable_testing():
    config = ProjectConfig(project_name="TestProject", enable_testing=True)
    cmake_content = generate_cmake(config)
    assert "enable_testing()" in cmake_content
    assert "add_subdirectory(tests)" in cmake_content


def test_custom_install_path():
    config = ProjectConfig(project_name="TestProject",
                           install_path="custom_bin")
    cmake_content = generate_cmake(config)
    assert "install(TARGETS TestProject DESTINATION custom_bin)" in cmake_content


def test_subdirectories():
    config = ProjectConfig(
        project_name="TestProject",
        subdirs=["module1", "module2"]
    )
    cmake_content = generate_cmake(config)
    assert 'add_subdirectory(module1)' in cmake_content
    assert 'add_subdirectory(module2)' in cmake_content


def test_different_cpp_standard():
    config = ProjectConfig(
        project_name="TestProject",
        cpp_standard="17"
    )
    cmake_content = generate_cmake(config)
    assert "set(CMAKE_CXX_STANDARD 17)" in cmake_content
    assert "set(CMAKE_CXX_STANDARD_REQUIRED True)" in cmake_content


def test_no_dependencies():
    config = ProjectConfig(
        project_name="TestProject",
        dependencies=[]
    )
    cmake_content = generate_cmake(config)
    assert "find_package" not in cmake_content
    assert "target_link_libraries" not in cmake_content


def test_custom_sources():
    config = ProjectConfig(
        project_name="TestProject",
        sources="src/**/*.cpp"
    )
    cmake_content = generate_cmake(config)
    assert 'file(GLOB_RECURSE SOURCES "src/**/*.cpp")' in cmake_content


def test_multiple_compiler_and_linker_flags():
    config = ProjectConfig(
        project_name="TestProject",
        compiler_flags=["-O2", "-g"],
        linker_flags=["-lpthread", "-lm"]
    )
    cmake_content = generate_cmake(config)
    assert "add_compile_options(-O2 -g)" in cmake_content
    assert "add_link_options(-lpthread -lm)" in cmake_content


def test_install_path():
    config = ProjectConfig(
        project_name="TestProject",
        install_path="custom/install/path"
    )
    cmake_content = generate_cmake(config)
    assert 'install(TARGETS TestProject DESTINATION custom/install/path)' in cmake_content


def test_enable_testing_with_subdirectory():
    config = ProjectConfig(
        project_name="TestProject",
        enable_testing=True
    )
    cmake_content = generate_cmake(config)
    assert "enable_testing()" in cmake_content
    assert "add_subdirectory(tests)" in cmake_content
