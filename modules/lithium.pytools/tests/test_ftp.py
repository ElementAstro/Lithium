import pytest
from ftplib import FTP, error_perm, all_errors
from unittest.mock import patch, MagicMock
from ftp import FTPClient

@pytest.fixture
def ftp_client():
    return FTPClient(host='test_host', username='test_user', password='test_pass')

def test_connect_success(ftp_client):
    with patch.object(FTP, 'connect', return_value=None), \
         patch.object(FTP, 'login', return_value=None):
        assert ftp_client.connect() is True
        assert ftp_client._is_connected is True

def test_connect_failure(ftp_client):
    with patch.object(FTP, 'connect', side_effect=all_errors):
        assert ftp_client.connect() is False
        assert ftp_client._is_connected is False

def test_disconnect_success(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client._is_connected = True
    ftp_client.disconnect()
    ftp_client.ftp.quit.assert_called_once()
    assert ftp_client._is_connected is False

def test_disconnect_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client._is_connected = True
    ftp_client.ftp.quit.side_effect = all_errors
    ftp_client.disconnect()
    ftp_client.ftp.quit.assert_called_once()
    assert ftp_client._is_connected is False

def test_list_files_success(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.dir = MagicMock(side_effect=lambda path, callback: callback('drwxr-xr-x 1 owner group 0 Jan 1 00:00 test_dir'))
    files = ftp_client.list_files()
    assert len(files) == 1
    assert files[0]['name'] == 'test_dir'
    assert files[0]['type'] == 'dir'

def test_list_files_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.dir.side_effect = error_perm
    files = ftp_client.list_files()
    assert files == []

def test_download_file_success(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.size.return_value = 100
    with patch('builtins.open', new_callable=MagicMock), \
         patch('os.path.getsize', return_value=0), \
         patch('tqdm.tqdm', return_value=MagicMock()):
        assert ftp_client.download_file('remote_file') is True

def test_download_file_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.size.side_effect = all_errors
    with patch('builtins.open', new_callable=MagicMock):
        assert ftp_client.download_file('remote_file') is False

def test_upload_file_success(ftp_client):
    ftp_client.ftp = MagicMock()
    with patch('builtins.open', new_callable=MagicMock), \
         patch('os.path.getsize', return_value=100), \
         patch('tqdm.tqdm', return_value=MagicMock()):
        assert ftp_client.upload_file('local_file') is True

def test_upload_file_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.storbinary.side_effect = all_errors
    with patch('builtins.open', new_callable=MagicMock):
        assert ftp_client.upload_file('local_file') is False

def test_delete_file_success(ftp_client):
    ftp_client.ftp = MagicMock()
    assert ftp_client.delete_file('test_file') is True
    ftp_client.ftp.delete.assert_called_once_with('test_file')

def test_delete_file_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.delete.side_effect = all_errors
    assert ftp_client.delete_file('test_file') is False

def test_change_directory_success(ftp_client):
    ftp_client.ftp = MagicMock()
    assert ftp_client.change_directory('test_dir') is True
    ftp_client.ftp.cwd.assert_called_once_with('test_dir')

def test_change_directory_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.cwd.side_effect = all_errors
    assert ftp_client.change_directory('test_dir') is False

def test_make_directory_success(ftp_client):
    ftp_client.ftp = MagicMock()
    assert ftp_client.make_directory('test_dir') is True
    ftp_client.ftp.mkd.assert_called_once_with('test_dir')

def test_make_directory_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.mkd.side_effect = all_errors
    assert ftp_client.make_directory('test_dir') is False

def test_get_current_directory_success(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.pwd.return_value = '/test_dir'
    assert ftp_client.get_current_directory() == '/test_dir'

def test_get_current_directory_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.pwd.side_effect = all_errors
    assert ftp_client.get_current_directory() == ''

def test_rename_file_success(ftp_client):
    ftp_client.ftp = MagicMock()
    assert ftp_client.rename_file('old_name', 'new_name') is True
    ftp_client.ftp.rename.assert_called_once_with('old_name', 'new_name')

def test_rename_file_failure(ftp_client):
    ftp_client.ftp = MagicMock()
    ftp_client.ftp.rename.side_effect = all_errors
    assert ftp_client.rename_file('old_name', 'new_name') is False