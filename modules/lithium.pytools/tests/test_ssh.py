import pytest
from unittest.mock import patch, MagicMock
from ssh import SSHClient, SSHConnectionError, SSHCommandError, SFTPError, SSHError
import paramiko
@pytest.fixture
def ssh_client():
    return SSHClient(hostname="localhost", port=22, username="user", password="pass")

@patch("paramiko.SSHClient")
def test_connect_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.connect = MagicMock()
    mock_ssh_client.return_value.open_sftp = MagicMock()
    ssh_client.connect()
    mock_ssh_client.return_value.connect.assert_called_once()
    mock_ssh_client.return_value.open_sftp.assert_called_once()

@patch("paramiko.SSHClient")
def test_connect_authentication_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.connect.side_effect = paramiko.AuthenticationException
    with pytest.raises(SSHConnectionError):
        ssh_client.connect()

@patch("paramiko.SSHClient")
def test_connect_no_valid_connections(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.connect.side_effect = IndexError
    with pytest.raises(SSHConnectionError):
        ssh_client.connect()

@patch("paramiko.SSHClient")
def test_connect_ssh_exception(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.connect.side_effect = paramiko.SSHException
    with pytest.raises(SSHConnectionError):
        ssh_client.connect()

@patch("paramiko.SSHClient")
def test_connect_general_exception(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.connect.side_effect = Exception
    with pytest.raises(SSHConnectionError):
        ssh_client.connect()

@patch("paramiko.SSHClient")
def test_execute_command_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.exec_command.return_value = (MagicMock(), MagicMock(), MagicMock())
    mock_ssh_client.return_value.exec_command.return_value[1].read.return_value = b"output"
    mock_ssh_client.return_value.exec_command.return_value[2].read.return_value = b""
    mock_ssh_client.return_value.exec_command.return_value[1].channel.recv_exit_status.return_value = 0
    ssh_client.connect()
    output, error = ssh_client.execute_command("ls")
    assert output == "output"
    assert error == ""

@patch("paramiko.SSHClient")
def test_execute_command_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.exec_command.return_value = (MagicMock(), MagicMock(), MagicMock())
    mock_ssh_client.return_value.exec_command.return_value[1].read.return_value = b""
    mock_ssh_client.return_value.exec_command.return_value[2].read.return_value = b"error"
    mock_ssh_client.return_value.exec_command.return_value[1].channel.recv_exit_status.return_value = 1
    ssh_client.connect()
    with pytest.raises(SSHCommandError):
        ssh_client.execute_command("ls")

@patch("paramiko.SSHClient")
def test_upload_file_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.put = MagicMock()
    ssh_client.connect()
    ssh_client.upload_file("local.txt", "remote.txt")
    mock_ssh_client.return_value.open_sftp.return_value.put.assert_called_once_with("local.txt", "remote.txt")

@patch("paramiko.SSHClient")
def test_upload_file_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.put.side_effect = FileNotFoundError
    ssh_client.connect()
    with pytest.raises(SFTPError):
        ssh_client.upload_file("local.txt", "remote.txt")

@patch("paramiko.SSHClient")
def test_download_file_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.get = MagicMock()
    ssh_client.connect()
    ssh_client.download_file("remote.txt", "local.txt")
    mock_ssh_client.return_value.open_sftp.return_value.get.assert_called_once_with("remote.txt", "local.txt")

@patch("paramiko.SSHClient")
def test_download_file_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.get.side_effect = FileNotFoundError
    ssh_client.connect()
    with pytest.raises(SFTPError):
        ssh_client.download_file("remote.txt", "local.txt")

@patch("paramiko.SSHClient")
def test_list_remote_directory_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.listdir.return_value = ["file1.txt", "file2.txt"]
    ssh_client.connect()
    files = ssh_client.list_remote_directory("remote_dir")
    assert files == ["file1.txt", "file2.txt"]

@patch("paramiko.SSHClient")
def test_list_remote_directory_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.listdir.side_effect = FileNotFoundError
    ssh_client.connect()
    with pytest.raises(SFTPError):
        ssh_client.list_remote_directory("remote_dir")

@patch("paramiko.SSHClient")
def test_create_remote_directory_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.mkdir = MagicMock()
    ssh_client.connect()
    ssh_client.create_remote_directory("remote_dir")
    mock_ssh_client.return_value.open_sftp.return_value.mkdir.assert_called_once_with("remote_dir")

@patch("paramiko.SSHClient")
def test_create_remote_directory_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.mkdir.side_effect = Exception
    ssh_client.connect()
    with pytest.raises(SFTPError):
        ssh_client.create_remote_directory("remote_dir")

@patch("paramiko.SSHClient")
def test_delete_remote_file_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.remove = MagicMock()
    ssh_client.connect()
    ssh_client.delete_remote_file("remote.txt")
    mock_ssh_client.return_value.open_sftp.return_value.remove.assert_called_once_with("remote.txt")

@patch("paramiko.SSHClient")
def test_delete_remote_file_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.open_sftp.return_value.remove.side_effect = FileNotFoundError
    ssh_client.connect()
    with pytest.raises(SFTPError):
        ssh_client.delete_remote_file("remote.txt")

@patch("paramiko.SSHClient")
def test_close_success(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.close = MagicMock()
    mock_ssh_client.return_value.open_sftp.return_value.close = MagicMock()
    ssh_client.connect()
    ssh_client.close()
    mock_ssh_client.return_value.close.assert_called_once()
    mock_ssh_client.return_value.open_sftp.return_value.close.assert_called_once()

@patch("paramiko.SSHClient")
def test_close_failure(mock_ssh_client, ssh_client):
    mock_ssh_client.return_value.close.side_effect = Exception
    ssh_client.connect()
    with pytest.raises(SSHError):
        ssh_client.close()