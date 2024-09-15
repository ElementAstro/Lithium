import pytest
import sys
from unittest.mock import patch, MagicMock
from tools.package import main


@pytest.fixture
def mock_run_command():
    with patch('tools.package.run_command') as mock:
        yield mock


@pytest.fixture
def mock_is_package_installed():
    with patch('tools.package.is_package_installed') as mock:
        yield mock


@pytest.fixture
def mock_get_installed_version():
    with patch('tools.package.get_installed_version') as mock:
        yield mock


def test_check_package_installed(mock_is_package_installed, mock_get_installed_version, mock_run_command):
    mock_is_package_installed.return_value = True
    mock_get_installed_version.return_value = "1.0.0"
    testargs = ["package.py", "--check", "pytest"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_is_package_installed.assert_called_once_with("pytest")
    mock_get_installed_version.assert_called_once_with("pytest")


def test_check_package_not_installed(mock_is_package_installed, mock_run_command):
    mock_is_package_installed.return_value = False
    testargs = ["package.py", "--check", "pytest"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_is_package_installed.assert_called_once_with("pytest")


def test_install_package(mock_run_command):
    testargs = ["package.py", "--install", "pytest"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "install", "pytest"])


def test_install_package_with_version(mock_run_command):
    testargs = ["package.py", "--install", "pytest", "--version", "6.2.4"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "install", "pytest==6.2.4"])


def test_upgrade_package(mock_run_command):
    testargs = ["package.py", "--upgrade", "pytest"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "install", "--upgrade", "pytest"])


def test_uninstall_package(mock_run_command):
    testargs = ["package.py", "--uninstall", "pytest"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "uninstall", "-y", "pytest"])


def test_list_installed_packages(mock_run_command):
    mock_run_command.return_value = "pytest 6.2.4"
    testargs = ["package.py", "--list-installed"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "list"])


def test_freeze_installed_packages(mock_run_command):
    mock_run_command.return_value = "pytest==6.2.4"
    testargs = ["package.py", "--freeze"]
    with patch.object(sys, 'argv', testargs):
        main()
    mock_run_command.assert_called_once_with(
        [sys.executable, "-m", "pip", "freeze"])
    with open("requirements.txt", "r") as f:
        content = f.read()
    assert content == "pytest==6.2.4"
