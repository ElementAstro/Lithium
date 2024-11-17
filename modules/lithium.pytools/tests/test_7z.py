import pytest
from pathlib import Path
from unittest.mock import patch, MagicMock
from seven import SevenZipWrapper, SevenZipValidationError, SevenZipCompressionError, SevenZipExtractionError, SevenZipListError, SevenZipTestError, SevenZipError
import subprocess
@pytest.fixture
def seven_zip():
    return SevenZipWrapper(executable="7z")

def test_init_valid_executable(seven_zip):
    assert seven_zip.executable == "7z"

def test_init_invalid_executable():
    with patch("shutil.which", return_value=None):
        with pytest.raises(SevenZipValidationError):
            SevenZipWrapper(executable="invalid_executable")

def test_validate_files_exist(seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip._validate_files_exist(["file1.txt", "file2.txt"])

def test_validate_files_exist_invalid(seven_zip):
    with patch("pathlib.Path.exists", side_effect=[True, False]):
        with pytest.raises(SevenZipValidationError):
            seven_zip._validate_files_exist(["file1.txt", "file2.txt"])

def test_validate_archive_exists(seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip._validate_archive_exists("archive.7z")

def test_validate_archive_exists_invalid(seven_zip):
    with patch("pathlib.Path.exists", return_value=False):
        with pytest.raises(SevenZipValidationError):
            seven_zip._validate_archive_exists("archive.7z")

@patch("subprocess.run")
def test_compress(mock_run, seven_zip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Success", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip.compress(["file1.txt"], "archive.7z")
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_compress_failure(mock_run, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(SevenZipCompressionError):
            seven_zip.compress(["file1.txt"], "archive.7z")


@patch("subprocess.run")
def test_list_contents(mock_run, seven_zip):
    mock_run.return_value = MagicMock(returncode=0, stdout="file1.txt\nfile2.txt", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        contents = seven_zip.list_contents("archive.7z")
    assert contents == "file1.txt\nfile2.txt"
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_list_contents_failure(mock_run, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(SevenZipListError):
            seven_zip.list_contents("archive.7z")

@patch("subprocess.run")
def test_test_archive(mock_run, seven_zip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Everything is Ok", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        is_valid = seven_zip.test_archive("archive.7z")
    assert is_valid
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_test_archive_failure(mock_run, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(SevenZipTestError):
            seven_zip.test_archive("archive.7z")

@patch("pathlib.Path.unlink")
def test_delete_archive(mock_unlink, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip.delete_archive("archive.7z")
    mock_unlink.assert_called_once()

@patch("pathlib.Path.unlink", side_effect=Exception("Error"))
def test_delete_archive_failure(mock_unlink, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(SevenZipError):
            seven_zip.delete_archive("archive.7z")

@patch("subprocess.run")
def test_update_archive_add(mock_run, seven_zip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Success", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip.update_archive("archive.7z", ["file1.txt"], add=True, delete=False)
    mock_run.assert_called_once()

@patch("subprocess.run")
def test_update_archive_delete(mock_run, seven_zip):
    mock_run.return_value = MagicMock(returncode=0, stdout="Success", stderr="")
    with patch("pathlib.Path.exists", return_value=True):
        seven_zip.update_archive("archive.7z", ["file1.txt"], add=False, delete=True)
    mock_run.assert_called_once()

@patch("subprocess.run", side_effect=subprocess.CalledProcessError(1, "cmd"))
def test_update_archive_failure(mock_run, seven_zip):
    with patch("pathlib.Path.exists", return_value=True):
        with pytest.raises(SevenZipError):
            seven_zip.update_archive("archive.7z", ["file1.txt"], add=True, delete=False)