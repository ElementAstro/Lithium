import pytest
import json
import asyncio
from unittest.mock import patch, mock_open, AsyncMock
from pathlib import Path
from ..hotspot import HotspotManager, HotspotConfig, Platform, AuthType, EncryptionType

# FILE: modules/lithium.pytools/tools/test_hotspot.py


@pytest.fixture
def hotspot_manager():
    with patch('platform.system', return_value='Linux'):
        return HotspotManager()


def test_init(hotspot_manager):
    assert hotspot_manager.platform == Platform.LINUX
    assert hotspot_manager.config_path == Path.home() / ".hotspot"
    assert hotspot_manager.saved_configs == {}


@patch('builtins.open', new_callable=mock_open, read_data='{}')
def test_load_config(mock_file, hotspot_manager):
    hotspot_manager._load_config()
    mock_file.assert_called_once_with(
        hotspot_manager.config_path / "config.json")
    assert hotspot_manager.saved_configs == {}


@patch('builtins.open', new_callable=mock_open)
def test_save_config(mock_file, hotspot_manager):
    hotspot_manager.saved_configs = {"test": "config"}
    hotspot_manager._save_config()
    mock_file.assert_called_once_with(
        hotspot_manager.config_path / "config.json", "w")
    mock_file().write.assert_called_once_with(
        json.dumps({"test": "config"}, indent=4))


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_run_command_async(mock_subprocess, hotspot_manager):
    mock_subprocess.return_value.communicate.return_value = (b'output', b'')
    stdout, stderr = await hotspot_manager._run_command_async(['echo', 'test'])
    assert stdout == 'output'
    assert stderr == ''


@patch('subprocess.run')
def test_run_command(mock_run, hotspot_manager):
    mock_run.return_value.stdout = 'output'
    result = hotspot_manager._run_command(['echo', 'test'])
    assert result == 'output'


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_start_linux(mock_subprocess, hotspot_manager):
    config = HotspotConfig(name="TestHotspot", password="password")
    await hotspot_manager._start_linux(config)
    mock_subprocess.assert_called_with(
        'nmcli', 'dev', 'wifi', 'hotspot', 'ifname', 'wlan0', 'ssid', 'TestHotspot', 'password', 'password',
        stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
    )


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_start_windows(mock_subprocess, hotspot_manager):
    with patch('platform.system', return_value='Windows'):
        hotspot_manager.platform = Platform.WINDOWS
        config = HotspotConfig(name="TestHotspot", password="password")
        await hotspot_manager._start_windows(config)
        mock_subprocess.assert_any_call(
            'netsh', 'wlan', 'set', 'hostednetwork', 'mode=allow', 'ssid=TestHotspot', 'key=password',
            stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
        )
        mock_subprocess.assert_any_call(
            'netsh', 'wlan', 'start', 'hostednetwork',
            stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
        )


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_stop(mock_subprocess, hotspot_manager):
    await hotspot_manager.stop()
    mock_subprocess.assert_called_with(
        'nmcli', 'connection', 'down', 'Hotspot',
        stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE
    )


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_status(mock_subprocess, hotspot_manager):
    mock_subprocess.return_value.communicate.return_value = (b'connected', b'')
    status = await hotspot_manager.status()
    assert status['status'] == 'running'


@patch('asyncio.create_subprocess_exec', new_callable=AsyncMock)
async def test_list_clients(mock_subprocess, hotspot_manager):
    mock_subprocess.return_value.communicate.return_value = (b'', b'')
    clients = await hotspot_manager.list_clients()
    assert clients == []


def test_save_profile(hotspot_manager):
    config = HotspotConfig(name="TestHotspot")
    hotspot_manager.save_profile("test_profile", config)
    assert hotspot_manager.saved_configs["test_profile"]["name"] == "TestHotspot"


def test_load_profile(hotspot_manager):
    config = HotspotConfig(name="TestHotspot")
    hotspot_manager.save_profile("test_profile", config)
    loaded_config = hotspot_manager.load_profile("test_profile")
    assert loaded_config.name == "TestHotspot"
