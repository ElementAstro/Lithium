import pytest
from unittest.mock import patch, MagicMock
from pathlib import Path
from ..atom_generator import generate_atom_module
import tempfile

# FILE: modules/lithium.pytools/tools/test_atom_generator.py


@pytest.fixture
def sample_header_file():
    # Create a temporary C++ header file
    header_content = """
    namespace TestNamespace {
        class TestClass {
        public:
            void testMethod();
        };
        void testFunction();
    }
    """
    with tempfile.NamedTemporaryFile(delete=False, suffix=".h") as temp_file:
        temp_file.write(header_content.encode('utf-8'))
        temp_file_path = Path(temp_file.name)
    yield temp_file_path
    temp_file_path.unlink()  # Clean up the file after the test


@pytest.fixture
def mock_parse_header_file():
    with patch('modules.lithium.pytools.tools.atom_generator.parse_header_file') as mock_parse:
        mock_parse.return_value = (
            {'TestNamespace::TestClass': ['testMethod']},
            ['TestNamespace::testFunction']
        )
        yield mock_parse


@pytest.fixture
def mock_logger():
    with patch('modules.lithium.pytools.tools.atom_generator.logger') as mock_log:
        yield mock_log


def test_generate_atom_module_single_file(sample_header_file, mock_parse_header_file, mock_logger):
    generate_atom_module([sample_header_file], log_level="DEBUG")
    mock_logger.info.assert_any_call("Generating ATOM_MODULE...\n")
    mock_logger.info.assert_any_call(
        "Registered method: TestNamespace::TestClass::testMethod")
    mock_logger.info.assert_any_call(
        "Registered global function: TestNamespace::testFunction")
    mock_logger.info.assert_any_call("ATOM_MODULE generation completed.\n")


def test_generate_atom_module_multiple_files(sample_header_file, mock_parse_header_file, mock_logger):
    generate_atom_module(
        [sample_header_file, sample_header_file], log_level="DEBUG")
    mock_logger.info.assert_any_call("Generating ATOM_MODULE...\n")
    # Ensure multiple calls for multiple files
    assert mock_logger.info.call_count >= 6


def test_generate_atom_module_with_whitelist(sample_header_file, mock_parse_header_file, mock_logger):
    generate_atom_module([sample_header_file], whitelist=[
                         'testMethod'], log_level="DEBUG")
    mock_logger.info.assert_any_call("Generating ATOM_MODULE...\n")
    mock_logger.info.assert_any_call(
        "Registered method: TestNamespace::TestClass::testMethod")
    mock_logger.info.assert_any_call("ATOM_MODULE generation completed.\n")


def test_generate_atom_module_with_blacklist(sample_header_file, mock_parse_header_file, mock_logger):
    generate_atom_module([sample_header_file], blacklist=[
                         'testMethod'], log_level="DEBUG")
    mock_logger.info.assert_any_call("Generating ATOM_MODULE...\n")
    mock_logger.info.assert_any_call("ATOM_MODULE generation completed.\n")
    assert not any(
        call for call in mock_logger.info.call_args_list if "Registered method" in call[0][0])


def test_generate_atom_module_output_to_file(sample_header_file, mock_parse_header_file, mock_logger):
    with tempfile.NamedTemporaryFile(delete=False, suffix=".cpp") as temp_file:
        output_path = Path(temp_file.name)
    generate_atom_module([sample_header_file],
                         output_file=output_path, log_level="DEBUG")
    with open(output_path, 'r') as f:
        generated_code = f.read()
    assert 'ATOM_MODULE(all_components, [](Component &component) {' in generated_code
    output_path.unlink()  # Clean up the file after the test


def test_generate_atom_module_output_to_console(sample_header_file, mock_parse_header_file, mock_logger):
    with patch('builtins.print') as mock_print:
        generate_atom_module([sample_header_file], log_level="DEBUG")
        mock_print.assert_any_call(
            'ATOM_MODULE(all_components, [](Component &component) {', end="")


def test_generate_atom_module_invalid_header_file(mock_parse_header_file, mock_logger):
    with pytest.raises(Exception):
        generate_atom_module(
            [Path('/invalid/path/to/header.h')], log_level="DEBUG")
    mock_logger.error.assert_called()


def test_generate_atom_module_missing_methods_or_functions(sample_header_file, mock_logger):
    with patch('modules.lithium.pytools.tools.atom_generator.parse_header_file', return_value=({}, [])):
        generate_atom_module([sample_header_file], log_level="DEBUG")
        mock_logger.info.assert_any_call("Generating ATOM_MODULE...\n")
        mock_logger.info.assert_any_call("ATOM_MODULE generation completed.\n")
        assert not any(
            call for call in mock_logger.info.call_args_list if "Registered method" in call[0][0])
        assert not any(
            call for call in mock_logger.info.call_args_list if "Registered global function" in call[0][0])
