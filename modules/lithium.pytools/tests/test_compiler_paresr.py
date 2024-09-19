# modules/lithium.pytools/tools/test_compiler_parser.py

import pytest
from unittest.mock import patch, mock_open, MagicMock
from pathlib import Path
import json
import csv
import xml.etree.ElementTree as ET
from ..tools.compiler_parser import (
    parse_gcc_clang_output,
    parse_msvc_output,
    parse_cmake_output,
    parse_output,
    write_to_csv,
    write_to_xml,
    process_file,
    main
)


def test_parse_gcc_clang_output():
    output = """
    gcc version 9.3.0
    file1.c:10:5: error: something went wrong
    file2.c:20:10: warning: something might be wrong
    """
    expected = {
        "version": "gcc version 9.3.0",
        "results": {
            "errors": [{"file": "file1.c", "line": 10, "column": 5, "message": "something went wrong", "severity": "error"}],
            "warnings": [{"file": "file2.c", "line": 20, "column": 10, "message": "something might be wrong", "severity": "warning"}],
            "info": []
        }
    }
    assert parse_gcc_clang_output(output) == expected


def test_parse_msvc_output():
    output = """
    Compiler Version 19.28.29336 for x64
    file1.c(10): error C2143: syntax error
    file2.c(20): warning C4996: 'strcpy': This function or variable may be unsafe.
    """
    expected = {
        "version": "Compiler Version 19.28.29336 for x64",
        "results": {
            "errors": [{"file": "file1.c", "line": 10, "code": "C2143", "message": "syntax error", "severity": "error"}],
            "warnings": [{"file": "file2.c", "line": 20, "code": "C4996", "message": "'strcpy': This function or variable may be unsafe.", "severity": "warning"}],
            "info": []
        }
    }
    assert parse_msvc_output(output) == expected


def test_parse_cmake_output():
    output = """
    cmake version 3.18.2
    CMake Error at CMakeLists.txt:10 (message):
      Something went wrong
    """
    expected = {
        "version": "cmake version 3.18.2",
        "results": {
            "errors": [{"file": "CMakeLists.txt", "line": 10, "message": "Something went wrong", "severity": "error"}],
            "warnings": [],
            "info": []
        }
    }
    assert parse_cmake_output(output) == expected


def test_parse_output():
    gcc_output = "gcc version 9.3.0\nfile1.c:10:5: error: something went wrong"
    msvc_output = "Compiler Version 19.28.29336 for x64\nfile1.c(10): error C2143: syntax error"
    cmake_output = "cmake version 3.18.2\nCMake Error at CMakeLists.txt:10 (message):\n  Something went wrong"

    assert parse_output('gcc', gcc_output)['version'] == "gcc version 9.3.0"
    assert parse_output('msvc', msvc_output)[
        'version'] == "Compiler Version 19.28.29336 for x64"
    assert parse_output('cmake', cmake_output)[
        'version'] == "cmake version 3.18.2"


@patch("builtins.open", new_callable=mock_open)
def test_write_to_csv(mock_file):
    data = [{"file": "file1.c", "line": 10, "column": 5, "type": "error",
             "code": "C2143", "message": "syntax error", "severity": "error"}]
    write_to_csv(data, "output.csv")
    mock_file.assert_called_once_with(
        "output.csv", 'w', newline='', encoding="utf-8")


@patch("xml.etree.ElementTree.ElementTree.write")
def test_write_to_xml(mock_write):
    data = [{"file": "file1.c", "line": 10, "type": "error",
             "code": "C2143", "message": "syntax error", "severity": "error"}]
    write_to_xml(data, "output.xml")
    mock_write.assert_called_once_with(
        "output.xml", encoding="utf-8", xml_declaration=True)


@patch("builtins.open", new_callable=mock_open, read_data="gcc version 9.3.0\nfile1.c:10:5: error: something went wrong")
def test_process_file(mock_file):
    result = process_file("gcc", "dummy_path")
    assert result["version"] == "gcc version 9.3.0"
    assert len(result["results"]["errors"]) == 1


@patch("argparse.ArgumentParser.parse_args")
@patch("builtins.open", new_callable=mock_open, read_data="gcc version 9.3.0\nfile1.c:10:5: error: something went wrong")
@patch("pathlib.Path.mkdir")
def test_main(mock_mkdir, mock_file, mock_args):
    mock_args.return_value = argparse.Namespace(
        compiler="gcc",
        file_paths=["dummy_path"],
        output_format="json",
        output_file="output.json",
        output_dir=".",
        filter=None,
        stats=False,
        concurrency=1
    )
    with patch("builtins.open", mock_open()) as mock_output_file:
        main()
        mock_output_file.assert_called_with(
            Path(".") / "output.json", 'w', encoding="utf-8")
