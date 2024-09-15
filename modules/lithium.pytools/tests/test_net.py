import pytest
from unittest.mock import patch, mock_open, MagicMock
from tools.net import verify_file_checksum, check_dotnet_installed, list_installed_dotnets, download_file_part, download_file, install_software, uninstall_dotnet


def test_verify_file_checksum():
    with patch("builtins.open", mock_open(read_data=b"test data")):
        assert verify_file_checksum(
            "dummy_path", "9e107d9d372bb6826bd81d3542a419d6", "md5") == True
        assert verify_file_checksum(
            "dummy_path", "wrong_checksum", "md5") == False


@patch("subprocess.run")
def test_check_dotnet_installed(mock_run):
    mock_run.return_value = MagicMock(returncode=0, stdout="v4\\Client")
    assert check_dotnet_installed("v4\\Client") == True
    mock_run.return_value = MagicMock(returncode=1, stdout="")
    assert check_dotnet_installed("v4\\Client") == False


@patch("subprocess.run")
def test_list_installed_dotnets(mock_run):
    mock_run.return_value = MagicMock(
        returncode=0, stdout="v4\\Client\nv4\\Full")
    with patch("builtins.print") as mock_print:
        list_installed_dotnets()
        mock_print.assert_any_call("Installed .NET Framework versions:")
        mock_print.assert_any_call("v4\\Client")
        mock_print.assert_any_call("v4\\Full")


@patch("requests.get")
def test_download_file_part(mock_get):
    mock_response = MagicMock()
    mock_response.content = b"part of file"
    mock_get.return_value = mock_response
    results = [None] * 2
    download_file_part("http://example.com", 0, 10, "dummy_file", 0, results)
    assert results[0] == b"part of file"


@patch("requests.head")
@patch("requests.get")
def test_download_file(mock_get, mock_head):
    mock_head.return_value = MagicMock(headers={"content-length": "20"})
    mock_response = MagicMock()
    mock_response.content = b"part of file"
    mock_get.return_value = mock_response
    with patch("builtins.open", mock_open()) as mock_file:
        download_file("http://example.com", "dummy_file", num_threads=2)
        mock_file().write.assert_called()


@patch("subprocess.run")
def test_install_software(mock_run):
    install_software("dummy_installer")
    mock_run.assert_called_with(
        ["start", "dummy_installer"], shell=True, check=True)


@patch("subprocess.run")
def test_uninstall_dotnet(mock_run):
    mock_run.return_value = MagicMock(returncode=0)
    uninstall_dotnet("v4\\Client")
    mock_run.assert_called_with(
        ["reg", "delete", "HKLM\\SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client", "/f"], capture_output=True, text=True, check=True)
