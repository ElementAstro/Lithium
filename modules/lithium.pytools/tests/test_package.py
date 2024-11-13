import pytest
import sys
from unittest.mock import patch, MagicMock
from tools.package import main
from unittest.mock import patch, MagicMock, mock_open
from pathlib import Path
from tools.package import PackageManager, Package, Source


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
    @pytest.fixture
    def mock_requests_get():
        with patch('tools.package.requests.get') as mock:
            yield mock

    @pytest.fixture
    def mock_open_file():
        with patch('builtins.open', mock_open(read_data="data")) as mock:
            yield mock

    @pytest.fixture
    def mock_tarfile_open():
        with patch('tools.package.tarfile.open') as mock:
            yield mock

    @pytest.fixture
    def mock_subprocess_run():
        with patch('tools.package.subprocess.run') as mock:
            yield mock

    @pytest.fixture
    def mock_logger():
        with patch('tools.package.logger') as mock:
            yield mock

    @pytest.fixture
    def package_manager():
        return PackageManager(config_file="test_packages.yaml")

    def test_load_config(package_manager, mock_open_file, mock_logger):
        mock_open_file.return_value.read.return_value = """
        version: "1.0"
        packages:
          test_package:
            sources:
              - url: "https://github.com/test/test_package"
                type: "github"
            version: "1.0.0"
            dependencies: []
        """
        package_manager.load_config()
        assert "test_package" in package_manager.packages
        assert package_manager.packages["test_package"].version == "1.0.0"

    def test_download_package(package_manager, mock_requests_get, mock_open_file, mock_logger):
        package = Package(
            name="test_package",
            sources=[Source(url="https://github.com/test/test_package", type="github")],
            version="1.0.0",
            dependencies=[],
            checksum=None,
            build_args=None
        )
        mock_requests_get.return_value.status_code = 200
        mock_requests_get.return_value.iter_content = lambda chunk_size: [b"data"]
        archive_path = package_manager.download_package(package)
        assert archive_path is not None

    def test_extract_package(package_manager, mock_tarfile_open, mock_logger):
        package = Package(
            name="test_package",
            sources=[Source(url="https://github.com/test/test_package", type="github")],
            version="1.0.0",
            dependencies=[],
            checksum=None,
            build_args=None
        )
        archive_path = Path("test_package-1.0.0.tar.gz")
        source_dir = package_manager.extract_package(archive_path, package)
        assert source_dir is not None

    def test_detect_build_system(package_manager):
        source_dir = Path("test_package")
        (source_dir / 'CMakeLists.txt').touch()
        build_system = package_manager.detect_build_system(source_dir)
        assert build_system == 'cmake'

    def test_build_package(package_manager, mock_subprocess_run, mock_logger):
        package = Package(
            name="test_package",
            sources=[Source(url="https://github.com/test/test_package", type="github")],
            version="1.0.0",
            dependencies=[],
            checksum=None,
            build_args=None
        )
        source_dir = Path("test_package")
        (source_dir / 'CMakeLists.txt').touch()
        package_manager.build_package(package, source_dir)
        mock_subprocess_run.assert_called()

    def test_run(package_manager, mock_subprocess_run, mock_logger):
        package = Package(
            name="test_package",
            sources=[Source(url="https://github.com/test/test_package", type="github")],
            version="1.0.0",
            dependencies=[],
            checksum=None,
            build_args=None
        )
        package_manager.packages["test_package"] = package
        package_manager.run(["test_package"])
        mock_subprocess_run.assert_called()