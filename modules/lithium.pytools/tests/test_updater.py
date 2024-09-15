import pytest
import json
import os
import zipfile
import shutil
import requests
import threading
import logging
from unittest.mock import patch, MagicMock
from pathlib import Path
from ..tools.updater import AutoUpdater


@pytest.fixture
def config():
    return {
        "url": "http://example.com/update",
        "install_dir": "/tmp/install_dir",
        "num_threads": 4,
        "custom_params": {
            "expected_hash": "dummyhash",
            "post_download": lambda: logging.info("Post download action"),
            "post_install": lambda: logging.info("Post install action")
        },
        "current_version": "1.0.0"
    }


@pytest.fixture
def updater(config):
    return AutoUpdater(config)


def test_init(updater, config):
    assert updater.url == config['url']
    assert updater.install_dir == Path(config['install_dir'])
    assert updater.num_threads == config['num_threads']
    assert updater.custom_params == config['custom_params']
    assert updater.temp_dir.exists()


@patch('requests.get')
def test_check_for_updates(mock_get, updater):
    mock_response = MagicMock()
    mock_response.json.return_value = {
        "version": "1.1.0",
        "download_url": "http://example.com/download"
    }
    mock_response.raise_for_status = MagicMock()
    mock_get.return_value = mock_response

    assert updater.check_for_updates() == True
    assert updater.latest_version == "1.1.0"
    assert updater.download_url == "http://example.com/download"

    mock_get.side_effect = requests.RequestException
    assert updater.check_for_updates() == False


def test_compare_versions(updater):
    updater.latest_version = "1.1.0"
    assert updater.compare_versions("1.0.0") == True
    assert updater.compare_versions("1.1.0") == False
    assert updater.compare_versions("1.2.0") == False


@patch('requests.get')
def test_download_file(mock_get, updater, tmp_path):
    mock_response = MagicMock()
    mock_response.iter_content = lambda chunk_size: [b"data"]
    mock_response.headers = {'content-length': '4'}
    mock_response.raise_for_status = MagicMock()
    mock_get.return_value = mock_response

    dest = tmp_path / "downloaded_file"
    updater.download_file("http://example.com/file", dest)
    assert dest.exists()
    assert dest.read_text() == "data"


def test_verify_file(updater, tmp_path):
    file_path = tmp_path / "file_to_verify"
    file_path.write_text("test content")
    expected_hash = "d8e8fca2dc0f896fd7cb4cb0031ba249"
    assert updater.verify_file(file_path, expected_hash) == True
    assert updater.verify_file(file_path, "wronghash") == False


def test_extract_zip(updater, tmp_path):
    zip_path = tmp_path / "test.zip"
    extract_to = tmp_path / "extracted"
    with zipfile.ZipFile(zip_path, 'w') as zipf:
        zipf.writestr("test.txt", "content")
    updater.extract_zip(zip_path, extract_to)
    assert (extract_to / "test.txt").exists()
    assert (extract_to / "test.txt").read_text() == "content"


def test_move_files(updater, tmp_path):
    src = tmp_path / "src"
    dest = tmp_path / "dest"
    src.mkdir()
    dest.mkdir()
    (src / "file.txt").write_text("content")
    updater.move_files(src, dest)
    assert (dest / "file.txt").exists()
    assert (dest / "file.txt").read_text() == "content"


def test_backup_files(updater, tmp_path):
    src = tmp_path / "src"
    backup_dir = tmp_path / "backup"
    src.mkdir()
    (src / "file.txt").write_text("content")
    updater.backup_files(src, backup_dir)
    assert (backup_dir / "file.txt").exists()
    assert (backup_dir / "file.txt").read_text() == "content"


def test_cleanup(updater):
    updater.temp_dir.mkdir(parents=True, exist_ok=True)
    (updater.temp_dir / "tempfile").write_text("content")
    updater.cleanup()
    assert not updater.temp_dir.exists()


def test_custom_post_download(updater):
    with patch.object(updater.custom_params, 'post_download', wraps=updater.custom_params['post_download']) as mock_post_download:
        updater.custom_post_download()
        mock_post_download.assert_called_once()


def test_custom_post_install(updater):
    with patch.object(updater.custom_params, 'post_install', wraps=updater.custom_params['post_install']) as mock_post_install:
        updater.custom_post_install()
        mock_post_install.assert_called_once()


def test_log_update(updater, tmp_path):
    updater.install_dir = tmp_path
    updater.log_update("1.0.0", "1.1.0")
    log_file = tmp_path / "update_log.txt"
    assert log_file.exists()
    assert "Updated from version 1.0.0 to 1.1.0" in log_file.read_text()


@patch('tools.updater.AutoUpdater.check_for_updates', return_value=True)
@patch('tools.updater.AutoUpdater.compare_versions', return_value=True)
@patch('tools.updater.AutoUpdater.download_file')
@patch('tools.updater.AutoUpdater.verify_file', return_value=True)
@patch('tools.updater.AutoUpdater.extract_zip')
@patch('tools.updater.AutoUpdater.backup_files')
@patch('tools.updater.AutoUpdater.move_files')
@patch('tools.updater.AutoUpdater.custom_post_download')
@patch('tools.updater.AutoUpdater.custom_post_install')
@patch('tools.updater.AutoUpdater.cleanup')
@patch('tools.updater.AutoUpdater.log_update')
def test_update(mock_log_update, mock_cleanup, mock_post_install, mock_post_download, mock_move_files, mock_backup_files, mock_extract_zip, mock_verify_file, mock_download_file, mock_compare_versions, mock_check_for_updates, updater):
    updater.update("1.0.0")
    mock_check_for_updates.assert_called_once()
    mock_compare_versions.assert_called_once_with("1.0.0")
    mock_download_file.assert_called_once()
    mock_verify_file.assert_called_once()
    mock_extract_zip.assert_called_once()
    mock_backup_files.assert_called_once()
    mock_move_files.assert_called_once()
    mock_post_download.assert_called_once()
    mock_post_install.assert_called_once()
    mock_cleanup.assert_called_once()
    mock_log_update.assert_called_once_with("1.0.0", updater.latest_version)
