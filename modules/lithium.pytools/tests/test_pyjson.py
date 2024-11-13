import sys
import pytest
import json
import os
from pathlib import Path
from ..tools.pyjson import flatten_json, load_json, print_json, save_json_to_yaml, query_json, validate_json, merge_json, diff_json, main
from unittest.mock import patch, mock_open


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


def test_main_minify(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--minify"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert json.loads(captured.out) == load_json(str(sample_json_file))


def test_main_format(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--format"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert json.loads(captured.out) == load_json(str(sample_json_file))


def test_main_yaml(sample_json_file, tmp_path):
    output_file = tmp_path / "output.yaml"
    testargs = ["pyjson.py", str(sample_json_file), "--yaml", str(output_file)]
    with patch.object(sys, 'argv', testargs):
        main()
    assert output_file.exists()


def test_main_query(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--query", "nested.key"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert captured.out.strip() == '"value"'


def test_main_validate(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--validate"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert "is valid" in captured.out


def test_main_merge(tmp_path):
    file1 = tmp_path / "file1.json"
    file2 = tmp_path / "file2.json"
    data1 = {"a": 1}
    data2 = {"b": 2}
    with open(file1, 'w', encoding='utf-8') as f:
        json.dump(data1, f)
    with open(file2, 'w', encoding='utf-8') as f:
        json.dump(data2, f)
    testargs = ["pyjson.py", str(file1), str(file2), "--merge"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert json.loads(captured.out) == {"a": 1, "b": 2}


def test_main_diff(tmp_path, capsys):
    file1 = tmp_path / "file1.json"
    file2 = tmp_path / "file2.json"
    data1 = {"a": 1, "b": 2}
    data2 = {"a": 1, "b": 3, "c": 4}
    with open(file1, 'w', encoding='utf-8') as f:
        json.dump(data1, f)
    with open(file2, 'w', encoding='utf-8') as f:
        json.dump(data2, f)
    testargs = ["pyjson.py", str(file1), str(file2), "--diff"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert json.loads(captured.out) == {"b": 3, "c": 4}


def test_main_stats(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--stats"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert "Total Keys" in captured.out
    assert "Total Elements" in captured.out
    assert "Depth" in captured.out


def test_main_flatten(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--flatten"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    flattened = flatten_json(load_json(str(sample_json_file)))
    assert json.loads(captured.out) == flattened


def test_main_unflatten(sample_json_file, capsys):
    flattened = flatten_json(load_json(str(sample_json_file)))
    flattened_file = sample_json_file.with_name("flattened.json")
    with open(flattened_file, 'w', encoding='utf-8') as f:
        json.dump(flattened, f)
    testargs = ["pyjson.py", str(flattened_file), "--unflatten"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    assert json.loads(captured.out) == load_json(str(sample_json_file))


def test_main_remove_key(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file), "--remove-key", "name"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    data = load_json(str(sample_json_file))
    del data["name"]
    assert json.loads(captured.out) == data


def test_main_rename_key(sample_json_file, capsys):
    testargs = ["pyjson.py", str(sample_json_file),
                "--rename-key", "name", "new_name"]
    with patch.object(sys, 'argv', testargs):
        main()
    captured = capsys.readouterr()
    data = load_json(str(sample_json_file))
    data["new_name"] = data.pop("name")
    assert json.loads(captured.out) == data
