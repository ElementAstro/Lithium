import pytest
import json
import subprocess
import os
import shutil
from pathlib import Path
from tools.script import load_config, install_dependencies, execute_script, list_scripts, clean_project, run_tests, deploy_project


@pytest.fixture
def sample_config_file(tmp_path):
    config_data = {
        "dependencies": ["pytest"],
        "scripts": {
            "build": "echo Building...",
            "test": "echo Testing...",
            "deploy": "echo Deploying..."
        },
        "temp_dirs": ["temp"]
    }
    config_file = tmp_path / "project.json"
    with open(config_file, 'w', encoding='utf-8') as f:
        json.dump(config_data, f)
    return config_file


def test_load_config(sample_config_file):
    config = load_config(str(sample_config_file))
    assert config["dependencies"] == ["pytest"]
    assert config["scripts"]["build"] == "echo Building..."
    assert config["temp_dirs"] == ["temp"]


def test_install_dependencies(monkeypatch):
    def mock_run(cmd, check):
        assert cmd == [sys.executable, "-m", "pip", "install", "pytest"]
    monkeypatch.setattr(subprocess, "run", mock_run)
    install_dependencies(["pytest"])


def test_execute_script(monkeypatch):
    def mock_run(cmd, shell, check):
        assert cmd == "echo Building..."
    monkeypatch.setattr(subprocess, "run", mock_run)
    execute_script("echo Building...")


def test_list_scripts(capsys):
    scripts = {
        "build": "echo Building...",
        "test": "echo Testing..."
    }
    list_scripts(scripts)
    captured = capsys.readouterr()
    assert "build: echo Building..." in captured.out
    assert "test: echo Testing..." in captured.out


def test_clean_project(tmp_path):
    temp_dir = tmp_path / "temp"
    temp_dir.mkdir()
    assert temp_dir.exists()
    clean_project([str(temp_dir)])
    assert not temp_dir.exists()


def test_run_tests(monkeypatch):
    def mock_run(cmd, shell, check):
        assert cmd == "echo Testing..."
    monkeypatch.setattr(subprocess, "run", mock_run)
    run_tests("echo Testing...")


def test_deploy_project(monkeypatch):
    def mock_run(cmd, shell, check):
        assert cmd == "echo Deploying..."
    monkeypatch.setattr(subprocess, "run", mock_run)
    deploy_project("echo Deploying...")
