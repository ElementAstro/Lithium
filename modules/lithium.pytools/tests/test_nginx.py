import pytest
import subprocess
import os
import platform
import sys
from unittest.mock import patch, mock_open, MagicMock
from ..nginx import install_nginx

# FILE: modules/lithium.pytools/tools/test_nginx.py


@pytest.fixture
def mock_platform():
    with patch('platform.system') as mock_platform:
        yield mock_platform


@pytest.fixture
def mock_os_path_isfile():
    with patch('os.path.isfile') as mock_isfile:
        yield mock_isfile


@pytest.fixture
def mock_subprocess_run():
    with patch('subprocess.run') as mock_run:
        yield mock_run


@pytest.fixture
def mock_sys_exit():
    with patch('sys.exit') as mock_exit:
        yield mock_exit


def test_install_nginx_already_installed(mock_subprocess_run, mock_platform):
    mock_platform.return_value = 'Linux'
    mock_subprocess_run.return_value = MagicMock(returncode=0)

    install_nginx()

    mock_subprocess_run.assert_called_once_with(
        ["/usr/sbin/nginx", "-v"], stderr=subprocess.PIPE, check=True
    )


def test_install_nginx_not_installed_debian(mock_subprocess_run, mock_platform, mock_os_path_isfile):
    mock_platform.return_value = 'Linux'
    mock_subprocess_run.side_effect = [
        subprocess.CalledProcessError(1, 'cmd'), MagicMock(returncode=0)]
    mock_os_path_isfile.side_effect = lambda path: path == "/etc/debian_version"

    install_nginx()

    mock_subprocess_run.assert_any_call(
        ["/usr/sbin/nginx", "-v"], stderr=subprocess.PIPE, check=True
    )
    mock_subprocess_run.assert_any_call(
        "sudo apt-get update && sudo apt-get install nginx -y", shell=True, check=True
    )


def test_install_nginx_not_installed_redhat(mock_subprocess_run, mock_platform, mock_os_path_isfile):
    mock_platform.return_value = 'Linux'
    mock_subprocess_run.side_effect = [
        subprocess.CalledProcessError(1, 'cmd'), MagicMock(returncode=0)]
    mock_os_path_isfile.side_effect = lambda path: path == "/etc/redhat-release"

    install_nginx()

    mock_subprocess_run.assert_any_call(
        ["/usr/sbin/nginx", "-v"], stderr=subprocess.PIPE, check=True
    )
    mock_subprocess_run.assert_any_call(
        "sudo yum update && sudo yum install nginx -y", shell=True, check=True
    )


def test_install_nginx_not_installed_unsupported_linux(mock_subprocess_run, mock_platform, mock_os_path_isfile, mock_sys_exit):
    mock_platform.return_value = 'Linux'
    mock_subprocess_run.side_effect = subprocess.CalledProcessError(1, 'cmd')
    mock_os_path_isfile.return_value = False

    install_nginx()

    mock_subprocess_run.assert_called_once_with(
        ["/usr/sbin/nginx", "-v"], stderr=subprocess.PIPE, check=True
    )
    mock_sys_exit.assert_called_once_with(1)


def test_install_nginx_not_installed_windows(mock_subprocess_run, mock_platform, mock_sys_exit):
    mock_platform.return_value = 'Windows'
    mock_subprocess_run.side_effect = subprocess.CalledProcessError(1, 'cmd')

    install_nginx()

    mock_subprocess_run.assert_called_once_with(
        ["C:\\nginx\\nginx.exe", "-v"], stderr=subprocess.PIPE, check=True
    )
    mock_sys_exit.assert_called_once_with(1)


def test_install_nginx_not_installed_unsupported_platform(mock_subprocess_run, mock_platform, mock_sys_exit):
    mock_platform.return_value = 'Darwin'
    mock_subprocess_run.side_effect = subprocess.CalledProcessError(1, 'cmd')

    install_nginx()

    mock_subprocess_run.assert_called_once_with(
        ["/usr/sbin/nginx", "-v"], stderr=subprocess.PIPE, check=True
    )
    mock_sys_exit.assert_called_once_with(1)
