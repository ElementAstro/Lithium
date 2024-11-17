import signal
import pytest
from unittest.mock import patch, MagicMock, mock_open
import os
import sys
import subprocess
import psutil
from daemon import DaemonProcess, write_pid, read_pid, is_daemon_running, stop_daemon, start_daemon, status_daemon, DEFAULT_CONFIG

@pytest.fixture
def config():
    return {
        "process_name": "python",
        "script_path": "target_script.py",
        "restart_interval": 5,
        "cpu_threshold": 80,
        "memory_threshold": 500,
        "max_restarts": 3,
        "monitor_interval": 5
    }

@pytest.fixture
def daemon_process(config):
    return DaemonProcess(config)

def test_init_valid_config(daemon_process):
    assert daemon_process.config["process_name"] == "python"
    assert daemon_process.config["script_path"] == "target_script.py"

def test_init_invalid_script_path(config):
    config["script_path"] = "invalid_script.py"
    with pytest.raises(FileNotFoundError):
        DaemonProcess(config)

@patch("subprocess.Popen")
def test_start_target_process(mock_popen, daemon_process):
    mock_popen.return_value.pid = 1234
    daemon_process.start_target_process()
    mock_popen.assert_called_once_with(
        [sys.executable, "target_script.py"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    assert daemon_process.process.pid == 1234

@patch("subprocess.Popen")
def test_is_process_running(mock_popen, daemon_process):
    mock_popen.return_value.poll.return_value = None
    daemon_process.start_target_process()
    assert daemon_process.is_process_running() is True

    mock_popen.return_value.poll.return_value = 1
    assert daemon_process.is_process_running() is False

@patch("psutil.Process")
@patch("subprocess.Popen")
def test_monitor_process_health(mock_popen, mock_psutil, daemon_process):
    mock_popen.return_value.pid = 1234
    mock_psutil.return_value.cpu_percent.return_value = 50
    mock_psutil.return_value.memory_info.return_value.rss = 400 * 1024 * 1024
    daemon_process.start_target_process()
    daemon_process.monitor_process_health()
    mock_psutil.assert_called_once_with(1234)

@patch("psutil.Process")
@patch("subprocess.Popen")
def test_monitor_process_health_exceed_thresholds(mock_popen, mock_psutil, daemon_process):
    mock_popen.return_value.pid = 1234
    mock_psutil.return_value.cpu_percent.return_value = 90
    mock_psutil.return_value.memory_info.return_value.rss = 600 * 1024 * 1024
    daemon_process.start_target_process()
    with patch.object(daemon_process, 'restart_process') as mock_restart:
        daemon_process.monitor_process_health()
        mock_restart.assert_called()

@patch("subprocess.Popen")
def test_restart_process(mock_popen, daemon_process):
    mock_popen.return_value.poll.return_value = None
    daemon_process.start_target_process()
    daemon_process.restart_process()
    assert daemon_process.restart_count == 1

@patch("subprocess.Popen")
def test_monitor_loop(mock_popen, daemon_process):
    mock_popen.return_value.poll.return_value = None
    with patch.object(daemon_process, 'is_process_running', return_value=True):
        with patch.object(daemon_process, 'monitor_process_health'):
            with patch("time.sleep", return_value=None):
                with patch.object(daemon_process, 'cleanup'):
                    daemon_process.monitor_loop()
                    daemon_process.monitor_process_health.assert_called()

@patch("subprocess.Popen")
def test_cleanup(mock_popen, daemon_process):
    mock_popen.return_value.poll.return_value = None
    daemon_process.start_target_process()
    daemon_process.cleanup()
    mock_popen.return_value.terminate.assert_called_once()

@patch("builtins.open", new_callable=mock_open)
def test_write_pid(mock_open):
    with patch("os.getpid", return_value=1234):
        write_pid()
        mock_open.assert_called_once_with("/tmp/daemon.pid", 'w', encoding='utf-8')
        mock_open().write.assert_called_once_with("1234")

@patch("builtins.open", new_callable=mock_open, read_data="1234")
def test_read_pid(mock_open):
    pid = read_pid()
    assert pid == 1234

@patch("psutil.pid_exists", return_value=True)
@patch("psutil.Process")
@patch("builtins.open", new_callable=mock_open, read_data="1234")
def test_is_daemon_running(mock_open, mock_psutil, mock_pid_exists, config):
    mock_psutil.return_value.name.return_value = "python"
    assert is_daemon_running(config) is True

@patch("psutil.Process")
@patch("builtins.open", new_callable=mock_open, read_data="1234")
def test_stop_daemon(mock_open, mock_psutil):
    mock_psutil.return_value.send_signal = MagicMock()
    mock_psutil.return_value.wait = MagicMock()
    stop_daemon()
    mock_psutil.return_value.send_signal.assert_called_once_with(signal.SIGTERM)

@patch("os.fork", side_effect=[0, 0])
@patch("os.setsid")
@patch("builtins.open", new_callable=mock_open)
@patch("daemon.DaemonProcess")
def test_start_daemon(mock_daemon_process, mock_open, mock_setsid, mock_fork, config):
    start_daemon(config)
    mock_daemon_process.assert_called_once_with(config)

@patch("psutil.pid_exists", return_value=True)
@patch("psutil.Process")
@patch("builtins.open", new_callable=mock_open, read_data="1234")
def test_status_daemon(mock_open, mock_psutil, mock_pid_exists):
    mock_psutil.return_value.name.return_value = "python"
    with patch("builtins.print") as mock_print:
        status_daemon()
        mock_print.assert_called_with("Daemon is running, PID: 1234")
        @patch("daemon.DaemonProcess.start_target_process")
        def test_monitor_loop_process_not_running(mock_start, daemon_process):
            with patch.object(daemon_process, 'is_process_running', return_value=False):
                with patch.object(daemon_process, 'restart_process') as mock_restart:
                    with patch("time.sleep", return_value=None):
                        with pytest.raises(SystemExit):
                            daemon_process.monitor_loop()
                    mock_restart.assert_called_once()
                    mock_start.assert_called_once()

        @patch("daemon.DaemonProcess.monitor_process_health")
        def test_monitor_loop_process_running(mock_monitor_health, daemon_process):
            with patch.object(daemon_process, 'is_process_running', return_value=True):
                with patch("time.sleep", return_value=None):
                    with patch.object(daemon_process, 'cleanup'):
                        # To prevent an infinite loop, run the loop only once
                        with patch("builtins.iter", return_value=range(1)):
                            daemon_process.monitor_loop()
                    mock_monitor_health.assert_called_once()

        @patch("psutil.Process")
        @patch("daemon.DaemonProcess.start_target_process")
        def test_monitor_process_health_cpu_exceeds(mock_start, mock_psutil, daemon_process):
            mock_proc = MagicMock()
            mock_psutil.return_value = mock_proc
            mock_proc.cpu_percent.return_value = 90
            mock_proc.memory_info.return_value.rss = 400 * 1024 * 1024
            daemon_process.start_target_process()
            daemon_process.monitor_process_health()
            mock_popen = daemon_process.process
            mock_proc.cpu_percent.assert_called_once_with(interval=1)
            daemon_process.restart_process.assert_called_once()

        @patch("psutil.Process")
        @patch("daemon.DaemonProcess.start_target_process")
        def test_monitor_process_health_memory_exceeds(mock_start, mock_psutil, daemon_process):
            mock_proc = MagicMock()
            mock_psutil.return_value = mock_proc
            mock_proc.cpu_percent.return_value = 50
            mock_proc.memory_info.return_value.rss = 600 * 1024 * 1024
            daemon_process.start_target_process()
            daemon_process.monitor_process_health()
            mock_popen = daemon_process.process
            mock_proc.cpu_percent.assert_called_once_with(interval=1)
            daemon_process.restart_process.assert_called_once()

        @patch("psutil.Process", side_effect=psutil.NoSuchProcess(pid=1234))
        def test_monitor_process_health_no_such_process(mock_psutil, daemon_process):
            daemon_process.start_target_process()
            with patch.object(daemon_process, 'restart_process') as mock_restart:
                daemon_process.monitor_process_health()
                mock_restart.assert_called_once()

        @patch("psutil.Process", side_effect=psutil.AccessDenied(pid=1234))
        def test_monitor_process_health_access_denied(mock_psutil, daemon_process, caplog):
            daemon_process.start_target_process()
            daemon_process.monitor_process_health()
            assert "Access denied when accessing process information." in caplog.text

        def test_is_process_running_none(daemon_process):
            daemon_process.process = None
            assert not daemon_process.is_process_running()

        @patch("psutil.Process")
        def test_is_process_running_true(mock_psutil, daemon_process):
            mock_proc = MagicMock()
            mock_proc.poll.return_value = None
            daemon_process.process = mock_proc
            assert daemon_process.is_process_running() is True
            mock_proc.poll.assert_called_once()

        @patch("psutil.Process")
        def test_is_process_running_false(mock_psutil, daemon_process):
            mock_proc = MagicMock()
            mock_proc.poll.return_value = 1
            daemon_process.process = mock_proc
            assert daemon_process.is_process_running() is False
            mock_proc.poll.assert_called_once()

        @patch("subprocess.Popen")
        def test_restart_process_below_max_restarts(mock_popen, daemon_process):
            daemon_process.restart_count = 2
            mock_popen.return_value.pid = 5678
            daemon_process.process = mock_popen.return_value
            daemon_process.restart_process()
            assert daemon_process.restart_count == 3
            mock_popen.assert_called_with(
                [sys.executable, "target_script.py"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )

        @patch("subprocess.Popen")
        def test_restart_process_max_restarts_reached(mock_popen, daemon_process):
            daemon_process.restart_count = 3
            with patch("daemon.DaemonProcess.cleanup") as mock_cleanup:
                with patch("sys.exit") as mock_exit:
                    daemon_process.restart_process()
                    mock_cleanup.assert_called_once()
                    mock_exit.assert_called_once_with("Daemon terminated: exceeded maximum restart count.")

        @patch("subprocess.Popen")
        def test_cleanup_terminate_running_process(mock_popen, daemon_process):
            mock_popen.return_value.poll.return_value = None
            daemon_process.process = mock_popen.return_value
            with patch.object(daemon_process, 'is_process_running', return_value=True):
                daemon_process.cleanup()
                mock_popen.return_value.terminate.assert_called_once()
                mock_popen.return_value.wait.assert_called_once_with(timeout=5)
                mock_popen.return_value.terminate.assert_called_once()

        @patch("subprocess.Popen")
        def test_cleanup_force_kill_process_on_timeout(mock_popen, daemon_process):
            mock_popen.return_value.poll.return_value = None
            daemon_process.process = mock_popen.return_value
            mock_popen.return_value.wait.side_effect = subprocess.TimeoutExpired(cmd='cmd', timeout=5)
            with patch.object(daemon_process, 'is_process_running', return_value=True):
                with patch("time.sleep", return_value=None):
                    daemon_process.cleanup()
                    mock_popen.return_value.terminate.assert_called_once()
                    mock_popen.return_value.kill.assert_called_once()
                    mock_popen.return_value.wait.assert_called()

        @patch("os.remove")
        @patch("os.path.exists", return_value=True)
        def test_cleanup_remove_pid_file(mock_exists, mock_remove, daemon_process):
            daemon_process.cleanup()
            mock_remove.assert_called_once_with("/tmp/daemon.pid")

        @patch("subprocess.Popen")
        def test_start_target_process_exception(mock_popen, daemon_process, caplog):
            mock_popen.side_effect = Exception("Start failed")
            with pytest.raises(Exception):
                daemon_process.start_target_process()
            assert "Failed to start target process: Start failed" in caplog.text

        @patch("subprocess.Popen", side_effect=subprocess.TimeoutExpired(cmd='cmd', timeout=5))
        def test_restart_process_force_kill_on_timeout(mock_popen, daemon_process, caplog):
            daemon_process.restart_count = 1
            daemon_process.process = mock_popen.return_value
            with patch.object(daemon_process, 'is_process_running', return_value=True):
                with pytest.raises(subprocess.TimeoutExpired):
                    daemon_process.restart_process()
            assert "Process PID: None force killed." in caplog.text