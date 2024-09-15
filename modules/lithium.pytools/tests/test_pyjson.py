import pytest
import json
import os
from pathlib import Path
from ..tools.pyjson import load_json, print_json, save_json_to_yaml, query_json, validate_json, merge_json, diff_json


@pytest.fixture
def sample_json_data():
    return {
        "name": "Test",
        "value": 123,
        "nested": {
            "key": "value"
        },
        "list": [1, 2, 3]
    }


@pytest.fixture
def sample_json_file(tmp_path, sample_json_data):
    file_path = tmp_path / "sample.json"
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(sample_json_data, f)
    return file_path


@pytest.fixture
def invalid_json_file(tmp_path):
    file_path = tmp_path / "invalid.json"
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write("{ invalid json }")
    return file_path


def test_load_json_valid(sample_json_file, sample_json_data):
    data = load_json(str(sample_json_file))
    assert data == sample_json_data


def test_load_json_invalid(invalid_json_file):
    with pytest.raises(SystemExit):
        load_json(str(invalid_json_file))


def test_print_json(capsys, sample_json_data):
    print_json(sample_json_data, minify=False)
    captured = capsys.readouterr()
    assert json.loads(captured.out) == sample_json_data

    print_json(sample_json_data, minify=True)
    captured = capsys.readouterr()
    assert json.loads(captured.out) == sample_json_data


def test_save_json_to_yaml(tmp_path, sample_json_data):
    output_file = tmp_path / "output.yaml"
    save_json_to_yaml(sample_json_data, str(output_file))
    assert output_file.exists()


def test_query_json(capsys, sample_json_data):
    query_json(sample_json_data, "nested.key")
    captured = capsys.readouterr()
    assert captured.out.strip() == '"value"'


def test_validate_json_valid(sample_json_file):
    validate_json(str(sample_json_file))


def test_validate_json_invalid(invalid_json_file):
    validate_json(str(invalid_json_file))


def test_merge_json(tmp_path, sample_json_data):
    file1 = tmp_path / "file1.json"
    file2 = tmp_path / "file2.json"
    data1 = {"a": 1}
    data2 = {"b": 2}
    with open(file1, 'w', encoding='utf-8') as f:
        json.dump(data1, f)
    with open(file2, 'w', encoding='utf-8') as f:
        json.dump(data2, f)
    merged_data = merge_json([str(file1), str(file2)])
    assert merged_data == {"a": 1, "b": 2}


def test_diff_json(capsys, tmp_path):
    file1 = tmp_path / "file1.json"
    file2 = tmp_path / "file2.json"
    data1 = {"a": 1, "b": 2}
    data2 = {"a": 1, "b": 3, "c": 4}
    with open(file1, 'w', encoding='utf-8') as f:
        json.dump(data1, f)
    with open(file2, 'w', encoding='utf-8') as f:
        json.dump(data2, f)
    diff_json(str(file1), str(file2))
    captured = capsys.readouterr()
    assert json.loads(captured.out) == {"b": 3, "c": 4}
