import pytest
from unittest.mock import Mock, patch
from ..tools.pacman import PacmanManager


@pytest.fixture
def pacman_manager():
    return PacmanManager()


def test_update_package_database_success(pacman_manager, monkeypatch):
    mock_run = Mock()
    mock_run.return_value.stdout = "Database updated successfully."
    monkeypatch.setattr("subprocess.run", mock_run)

    result = pacman_manager.update_package_database()
    assert result == "Database updated successfully."
    mock_run.assert_called_once_with(
        ['sudo', 'pacman', '-Sy'], check=True, text=True, capture_output=True)


def test_update_package_database_failure(pacman_manager, monkeypatch):
    mock_run = Mock()
    mock_run.side_effect = subprocess.CalledProcessError(
        1, 'pacman', stderr="Error updating database.")
    monkeypatch.setattr("subprocess.run", mock_run)

    result = pacman_manager.update_package_database()
    assert "Error: Error updating database." in result
    mock_run.assert_called_once_with(
        ['sudo', 'pacman', '-Sy'], check=True, text=True, capture_output=True)
