import pytest
from pathlib import Path
from unittest.mock import patch, MagicMock
from unzip import UnzipWrapper, UnzipValidationError, UnzipExtractionError, UnzipListError, UnzipIntegrityError, UnzipDeleteError, UnzipError
import subprocess

@pytest.fixture
def unzip():
    return UnzipWrapper(executable="unzip")

def test_init_valid_executable(unzip):
    assert unzip.executable == "unzip"

def test_init_invalid_executable():
    with patch("shutil.which", return_value=None):
        with pytest.raises(UnzipValidationError):
            UnzipWrapper(executable="invalid_executable")

def test_validate_archive_exists(unzip):
    with patch("pathlib.Path.exists", return_value=True):
        unzip._validate_archive_exists("archive.zip")

def test_validate_archive_exists_invalid(unzip):
    with patch("pathlib.Path.exists", return_value=False):
        with pytest.raises(UnzipValidationError):
            unzip._validate_archive_exists("archive.zip")

@patch("subprocess.run")
def test_extract(mock_run, unzip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Success", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        unzip.extract("archive.zip", "destination")
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_extract_failure(mock_run, unzip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(UnzipExtractionError):
            unzip.extract("archive.zip", "destination")

@patch("subprocess.run")
def test_list_contents(mock_run, unzip):
    mock_run.return_value = MagicMock(returncode=0, stdout="file1.txt\nfile2.txt", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        contents = unzip.list_contents("archive.zip")
    assert contents == "file1.txt\nfile2.txt"
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_list_contents_failure(mock_run, unzip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(UnzipListError):
            unzip.list_contents("archive.zip")

@patch("subprocess.run")
def test_test_integrity(mock_run, unzip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Everything is Ok", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        is_valid = unzip.test_integrity("archive.zip")
    assert is_valid
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_test_integrity_failure(mock_run, unzip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(UnzipIntegrityError):
            unzip.test_integrity("archive.zip")

@patch("pathlib.Path.unlink")
def test_delete_archive(mock_unlink, unzip):
    with patch("pathlib.Path.exists", return_value=True):
        unzip.delete_archive("archive.zip")
    mock_unlink.assert_called_once()

@patch("pathlib.Path.unlink", side_effect=Exception("Error"))
def test_delete_archive_failure(mock_unlink, unzip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(UnzipDeleteError):
            unzip.delete_archive("archive.zip")

@patch("pathlib.Path.exists", return_value=False)
def test_extract_nonexistent_archive(mock_exists, unzip):
    with pytest.raises(UnzipValidationError):
        unzip.extract("nonexistent.zip", "destination")

@patch("pathlib.Path.exists", return_value=True)
def test_list_contents_nonexistent_archive(mock_exists, unzip):
    with pytest.raises(UnzipListError):
        unzip.list_contents("nonexistent.zip")

@patch("pathlib.Path.exists", return_value=True)
def test_test_nonexistent_archive(mock_exists, unzip):
    with pytest.raises(UnzipIntegrityError):
        unzip.test_integrity("nonexistent.zip")

@patch("pathlib.Path.exists", return_value=True)
def test_delete_nonexistent_archive(mock_exists, unzip):
    with pytest.raises(UnzipDeleteError):
        unzip.delete_archive("nonexistent.zip")