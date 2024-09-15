import pytest
from unittest.mock import patch, MagicMock
from pathlib import Path
from ..tools.compiler import (
    detect_compilers, select_compiler, select_cpp_version, load_options_from_json, main, Compiler, CppVersion, CompilerType
)


def test_detect_compilers():
    with patch('..tools.compiler.find_command') as mock_find_command:
        mock_find_command.side_effect = lambda cmd: f"/usr/bin/{
            cmd}" if cmd in ["gcc", "clang", "cl"] else None
        compilers = detect_compilers()
        assert len(compilers) == 3
        assert compilers[0].name == "GCC"
        assert compilers[1].name == "Clang"
        assert compilers[2].name == "MSVC"


def test_select_compiler():
    compilers = [
        Compiler(name="GCC", command="gcc", compiler_type=CompilerType.GCC),
        Compiler(name="Clang", command="clang",
                 compiler_type=CompilerType.CLANG)
    ]
    with patch('builtins.input', return_value="1"):
        selected_compiler = select_compiler(compilers)
        assert selected_compiler.name == "GCC"


def test_select_cpp_version():
    with patch('builtins.input', return_value="3"):
        selected_version = select_cpp_version()
        assert selected_version == CppVersion.CPP11


def test_load_options_from_json():
    with patch('..tools.compiler.load_json', return_value={"compile_flags": ["-O2"], "link_flags": ["-lm"]}):
        options = load_options_from_json("dummy_path.json")
        assert options["compile_flags"] == ["-O2"]
        assert options["link_flags"] == ["-lm"]


def test_main_compile():
    with patch('argparse.ArgumentParser.parse_args', return_value=argparse.Namespace(
        source_files=[Path("source.cpp")],
        output=Path("output.o"),
        link=False,
        compiler="GCC",
        cpp_version="c++17",
        flags=None,
        compile_flags=None,
        link_flags=None,
        json_options=None
    )), patch('..tools.compiler.detect_compilers', return_value=[
        Compiler(name="GCC", command="gcc", compiler_type=CompilerType.GCC,
                 cpp_flags={CppVersion.CPP17: "-std=c++17"})
    ]), patch('..tools.compiler.subprocess.run') as mock_run:
        main()
        mock_run.assert_called_once_with(
            ["gcc", "-std=c++17", "-c", "source.cpp", "-o", "output.o"],
            check=True
        )


def test_main_link():
    with patch('argparse.ArgumentParser.parse_args', return_value=argparse.Namespace(
        source_files=[Path("source.o")],
        output=Path("output.exe"),
        link=True,
        compiler="GCC",
        cpp_version="c++17",
        flags=None,
        compile_flags=None,
        link_flags=None,
        json_options=None
    )), patch('..tools.compiler.detect_compilers', return_value=[
        Compiler(name="GCC", command="gcc", compiler_type=CompilerType.GCC,
                 cpp_flags={CppVersion.CPP17: "-std=c++17"})
    ]), patch('..tools.compiler.subprocess.run') as mock_run:
        main()
        mock_run.assert_called_once_with(
            ["gcc", "source.o", "-o", "output.exe"],
            check=True
        )
