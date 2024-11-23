import pytest
from unittest.mock import patch, mock_open
from pathlib import Path

from output import (
    pretty_print_json, pretty_print_yaml, pretty_print_toml,
    pretty_print_xml, pretty_print_csv, pretty_print_ini,
    validate_file, display_file
)


@pytest.fixture
def json_data():
    return '{"key": "value"}'


@pytest.fixture
def yaml_data():
    return 'key: value'


@pytest.fixture
def toml_data():
    return 'key = "value"'


@pytest.fixture
def xml_data():
    return '<root><key>value</key></root>'


@pytest.fixture
def csv_data():
    return 'key,value\nfoo,bar'


@pytest.fixture
def ini_data():
    return '[section]\nkey=value'


def test_pretty_print_json(json_data):
    with patch('builtins.open', mock_open(read_data=json_data)):
        pretty_print_json(json_data)


def test_pretty_print_yaml(yaml_data):
    with patch('builtins.open', mock_open(read_data=yaml_data)):
        pretty_print_yaml(yaml_data)


def test_pretty_print_toml(toml_data):
    with patch('builtins.open', mock_open(read_data=toml_data)):
        pretty_print_toml(toml_data)


def test_pretty_print_xml(xml_data):
    with patch('builtins.open', mock_open(read_data=xml_data)):
        pretty_print_xml(xml_data)


def test_pretty_print_csv(csv_data):
    with patch('builtins.open', mock_open(read_data=csv_data)):
        pretty_print_csv(csv_data)


def test_pretty_print_ini(ini_data):
    with patch('builtins.open', mock_open(read_data=ini_data)):
        pretty_print_ini(ini_data)


def test_validate_json(json_data):
    with patch('pathlib.Path.read_text', return_value=json_data):
        assert validate_file(Path('example.json'), 'json')


def test_validate_yaml(yaml_data):
    with patch('pathlib.Path.read_text', return_value=yaml_data):
        assert validate_file(Path('example.yaml'), 'yaml')


def test_validate_toml(toml_data):
    with patch('pathlib.Path.read_text', return_value=toml_data):
        assert validate_file(Path('example.toml'), 'toml')


def test_validate_xml(xml_data):
    with patch('pathlib.Path.read_text', return_value=xml_data):
        assert validate_file(Path('example.xml'), 'xml')


def test_validate_csv(csv_data):
    with patch('pathlib.Path.read_text', return_value=csv_data):
        assert validate_file(Path('example.csv'), 'csv')


def test_validate_ini(ini_data):
    with patch('pathlib.Path.read_text', return_value=ini_data):
        assert validate_file(Path('example.ini'), 'ini')


def test_display_file_json(json_data):
    with patch('pathlib.Path.read_text', return_value=json_data):
        display_file(Path('example.json'), 'json')


def test_display_file_yaml(yaml_data):
    with patch('pathlib.Path.read_text', return_value=yaml_data):
        display_file(Path('example.yaml'), 'yaml')


def test_display_file_toml(toml_data):
    with patch('pathlib.Path.read_text', return_value=toml_data):
        display_file(Path('example.toml'), 'toml')


def test_display_file_xml(xml_data):
    with patch('pathlib.Path.read_text', return_value=xml_data):
        display_file(Path('example.xml'), 'xml')


def test_display_file_csv(csv_data):
    with patch('pathlib.Path.read_text', return_value=csv_data):
        display_file(Path('example.csv'), 'csv')


def test_display_file_ini(ini_data):
    with patch('pathlib.Path.read_text', return_value=ini_data):
        display_file(Path('example.ini'), 'ini')
