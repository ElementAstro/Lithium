import pytest
from unittest.mock import patch, MagicMock
from paramiko import SFTPClient, SSHException
from sftp import SFTPClientWrapper

@pytest.fixture
def sftp_client():
    return SFTPClientWrapper(hostname='test_host', username='test_user', password='test_pass')

def test_connect_success(sftp_client):
    with patch.object(SFTPClientWrapper, 'connect', return_value=None):
        sftp_client.connect()
        assert sftp_client.client is not None
        assert sftp_client.sftp is not None

def test_connect_failure(sftp_client):
    with patch.object(SFTPClientWrapper, 'connect', side_effect=SSHException):
        with pytest.raises(SSHException):
            sftp_client.connect()

def test_upload_file_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.upload_file('local_path', 'remote_path')
    sftp_client.sftp.put.assert_called_once_with('local_path', 'remote_path')

def test_upload_file_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.put.side_effect = Exception
    sftp_client.upload_file('local_path', 'remote_path')
    sftp_client.sftp.put.assert_called_once_with('local_path', 'remote_path')

def test_download_file_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.download_file('remote_path', 'local_path')
    sftp_client.sftp.get.assert_called_once_with('remote_path', 'local_path')

def test_download_file_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.get.side_effect = Exception
    sftp_client.download_file('remote_path', 'local_path')
    sftp_client.sftp.get.assert_called_once_with('remote_path', 'local_path')

def test_upload_directory_success(sftp_client):
    sftp_client.sftp = MagicMock()
    with patch('os.walk', return_value=[('root', [], ['file1', 'file2'])]), \
         patch.object(SFTPClientWrapper, 'create_directory', return_value=None), \
         patch.object(SFTPClientWrapper, 'upload_file', return_value=None):
        sftp_client.upload_directory('local_dir', 'remote_dir')
        sftp_client.create_directory.assert_called()
        sftp_client.upload_file.assert_called()

def test_upload_directory_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    with patch('os.walk', return_value=[('root', [], ['file1', 'file2'])]), \
         patch.object(SFTPClientWrapper, 'create_directory', side_effect=Exception), \
         patch.object(SFTPClientWrapper, 'upload_file', return_value=None):
        sftp_client.upload_directory('local_dir', 'remote_dir')
        sftp_client.create_directory.assert_called()

def test_download_directory_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir_attr.return_value = [MagicMock(filename='file1', st_mode=0), MagicMock(filename='dir1', st_mode=stat.S_IFDIR)]
    with patch('os.makedirs', return_value=None), \
         patch.object(SFTPClientWrapper, 'download_file', return_value=None), \
         patch.object(SFTPClientWrapper, 'download_directory', return_value=None):
        sftp_client.download_directory('remote_dir', 'local_dir')
        sftp_client.download_file.assert_called()
        sftp_client.download_directory.assert_called()

def test_download_directory_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir_attr.side_effect = Exception
    with patch('os.makedirs', return_value=None):
        sftp_client.download_directory('remote_dir', 'local_dir')
        sftp_client.sftp.listdir_attr.assert_called_once_with('remote_dir')

def test_create_directory_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.create_directory('remote_path')
    sftp_client.sftp.mkdir.assert_called_once_with('remote_path')

def test_create_directory_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.mkdir.side_effect = Exception
    sftp_client.create_directory('remote_path')
    sftp_client.sftp.mkdir.assert_called_once_with('remote_path')

def test_remove_directory_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir_attr.return_value = [MagicMock(filename='file1', st_mode=0), MagicMock(filename='dir1', st_mode=stat.S_IFDIR)]
    with patch.object(SFTPClientWrapper, 'remove_directory', return_value=None):
        sftp_client.remove_directory('remote_path')
        sftp_client.sftp.remove.assert_called()
        sftp_client.sftp.rmdir.assert_called_once_with('remote_path')

def test_remove_directory_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir_attr.side_effect = Exception
    sftp_client.remove_directory('remote_path')
    sftp_client.sftp.listdir_attr.assert_called_once_with('remote_path')

def test_get_file_info_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.return_value = MagicMock(st_size=100, st_mtime=1000, st_mode=stat.S_IFREG)
    sftp_client.get_file_info('remote_path')
    sftp_client.sftp.stat.assert_called_once_with('remote_path')

def test_get_file_info_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.side_effect = Exception
    sftp_client.get_file_info('remote_path')
    sftp_client.sftp.stat.assert_called_once_with('remote_path')

def test_resume_upload_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.return_value = MagicMock(st_size=50)
    with patch('os.path.getsize', return_value=100), \
         patch('builtins.open', new_callable=MagicMock):
        sftp_client.resume_upload('local_path', 'remote_path')
        sftp_client.sftp.putfo.assert_called()

def test_resume_upload_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.side_effect = Exception
    with patch('os.path.getsize', return_value=100), \
         patch('builtins.open', new_callable=MagicMock):
        sftp_client.resume_upload('local_path', 'remote_path')
        sftp_client.sftp.putfo.assert_not_called()

def test_list_files_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir.return_value = ['file1', 'file2']
    files = sftp_client.list_files('remote_path')
    assert files == ['file1', 'file2']
    sftp_client.sftp.listdir.assert_called_once_with('remote_path')

def test_list_files_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.listdir.side_effect = Exception
    files = sftp_client.list_files('remote_path')
    assert files == []
    sftp_client.sftp.listdir.assert_called_once_with('remote_path')

def test_move_file_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.move_file('remote_src', 'remote_dest')
    sftp_client.sftp.rename.assert_called_once_with('remote_src', 'remote_dest')

def test_move_file_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.rename.side_effect = Exception
    sftp_client.move_file('remote_src', 'remote_dest')
    sftp_client.sftp.rename.assert_called_once_with('remote_src', 'remote_dest')

def test_delete_file_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.delete_file('remote_path')
    sftp_client.sftp.remove.assert_called_once_with('remote_path')

def test_delete_file_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.remove.side_effect = Exception
    sftp_client.delete_file('remote_path')
    sftp_client.sftp.remove.assert_called_once_with('remote_path')

def test_path_exists_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.return_value = True
    assert sftp_client.path_exists('remote_path') is True
    sftp_client.sftp.stat.assert_called_once_with('remote_path')

def test_path_exists_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.sftp.stat.side_effect = FileNotFoundError
    assert sftp_client.path_exists('remote_path') is False
    sftp_client.sftp.stat.assert_called_once_with('remote_path')

def test_disconnect_success(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.client = MagicMock()
    sftp_client.disconnect()
    sftp_client.sftp.close.assert_called_once()
    sftp_client.client.close.assert_called_once()

def test_disconnect_failure(sftp_client):
    sftp_client.sftp = MagicMock()
    sftp_client.client = MagicMock()
    sftp_client.sftp.close.side_effect = Exception
    sftp_client.disconnect()
    sftp_client.sftp.close.assert_called_once()
    sftp_client.client.close.assert_called_once()