import pytest
from ..tools.exebind import generate_pybind11_code

@pytest.fixture
def executable_name():
    return "test_executable"

@pytest.fixture
def command_info():
    return [
        ("-h", "Show this help message and exit"),
        ("--version", "Show version information"),
        ("--config <file>", "Specify configuration file")
    ]

def test_generate_pybind11_code_basic(executable_name, command_info):
    result = generate_pybind11_code(executable_name, command_info)
    
    # Check if the result contains the expected sections
    assert "#include <pybind11/pybind11.h>" in result
    assert "namespace py = pybind11;" in result
    assert f"class {executable_name}_Wrapper {{" in result
    assert f"PYBIND11_MODULE({executable_name}_bindings, m) {{" in result

    # Check if the result contains the generated functions
    for option, description in command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        assert f'std::string {function_name}' in result
        assert f'.def("{function_name}", &{executable_name}_Wrapper::{function_name}' in result

def test_generate_pybind11_code_empty_command_info(executable_name):
    result = generate_pybind11_code(executable_name, [])
    
    # Check if the result contains the expected sections
    assert "#include <pybind11/pybind11.h>" in result
    assert "namespace py = pybind11;" in result
    assert f"class {executable_name}_Wrapper {{" in result
    assert f"PYBIND11_MODULE({executable_name}_bindings, m) {{" in result

    # Check that no command-specific functions are generated
    assert "std::string get_" not in result

def test_generate_pybind11_code_special_characters(executable_name):
    special_command_info = [
        ("-@", "Special character option"),
        ("--complex-option", "Complex option with special characters: @#$%^&*()")
    ]
    result = generate_pybind11_code(executable_name, special_command_info)
    
    # Check if the result contains the generated functions
    for option, description in special_command_info:
        function_name = f'get_{option.lstrip("-").replace("-", "_")}'
        assert f'std::string {function_name}' in result
        assert f'.def("{function_name}", &{executable_name}_Wrapper::{function_name}' in result