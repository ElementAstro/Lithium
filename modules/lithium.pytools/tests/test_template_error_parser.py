import pytest
import json
from pathlib import Path
from tools.template_error_parser import (
    parse_cpp_template_error,
    generate_suggestions,
    extract_code_context,
    highlight_error_line,
    save_to_json,
    generate_error_tree
)


@pytest.fixture
def sample_error_message_gcc():
    return (
        "main.cpp:15:10: error: no matching function for call to 'foo'\n"
        "note: candidate function template not viable: no known conversion from 'int' to 'std::string' for 1st argument\n"
        "note: in instantiation of 'template<class T> void bar(T)':\n"
        "main.cpp:20:5:   required from here\n"
    )


@pytest.fixture
def sample_error_message_msvc():
    return (
        "main.cpp(15): error C2672: 'foo': no matching overloaded function found\n"
        "main.cpp(20): note: see reference to function template instantiation 'void bar<int>(T)' being compiled\n"
    )


@pytest.fixture
def expected_parsed_error_gcc():
    return {
        "main_error": {
            "file": "main.cpp",
            "line": "15",
            "column": "10",
            "message": "no matching function for call to 'foo'"
        },
        "nested_errors": [
            "in instantiation of 'template<class T> void bar(T)':"
        ],
        "error_details": [
            "no matching function",
            "no known conversion from 'int' to 'std::string' for 1st argument"
        ],
        "suggestions": [
            "Check the function signature and ensure template parameters match.",
            "Review the template parameters and constraints for compatibility."
        ]
    }


def test_parse_cpp_template_error_gcc(sample_error_message_gcc, expected_parsed_error_gcc):
    parsed_error = parse_cpp_template_error(
        sample_error_message_gcc, compiler='gcc')
    assert parsed_error == expected_parsed_error_gcc


def test_parse_cpp_template_error_msvc(sample_error_message_msvc):
    parsed_error = parse_cpp_template_error(
        sample_error_message_msvc, compiler='msvc')
    assert parsed_error["main_error"]["message"] == "no matching overloaded function found"
    assert "in instantiation of 'template<class T> void bar(T)'" in parsed_error[
        "nested_errors"]


def test_generate_suggestions(expected_parsed_error_gcc):
    error_info = expected_parsed_error_gcc.copy()
    error_info["suggestions"] = []
    generate_suggestions(error_info)
    assert error_info["suggestions"] == expected_parsed_error_gcc["suggestions"]


def test_extract_code_context(tmp_path):
    source_file = tmp_path / "main.cpp"
    source_code = (
        "#include <iostream>\n"
        "void foo(int x) {}\n"
        "int main() {\n"
        "    foo(42);\n"
        "    return 0;\n"
        "}\n"
    )
    source_file.write_text(source_code)
    context = extract_code_context(str(source_file), 4, context_lines=1)
    assert "foo(42);" in context


def test_highlight_error_line():
    context = (
        "#include <iostream>\n"
        "void foo(int x) {}\n"
        "int main() {\n"
        "    foo(42);\n"
        "    return 0;\n"
        "}\n"
    )
    highlighted_context = highlight_error_line(context, 3)
    assert "\033[1;33m    foo(42);\033[0m" in highlighted_context


def test_save_to_json(tmp_path, expected_parsed_error_gcc):
    output_file = tmp_path / "parsed_error.json"
    save_to_json(expected_parsed_error_gcc, str(output_file))
    with open(output_file, 'r') as f:
        saved_data = json.load(f)
    assert saved_data == expected_parsed_error_gcc


def test_generate_error_tree(tmp_path, expected_parsed_error_gcc):
    output_file = tmp_path / "error_tree"
    generate_error_tree(expected_parsed_error_gcc, str(
        output_file), format='png', view=False, save=True)
    assert Path(f"{output_file}.png").exists()
