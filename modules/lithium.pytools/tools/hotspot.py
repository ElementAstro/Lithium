# hotspot.py

"""
WiFi Hotspot Manager
A cross-platform tool for managing WiFi hotspots with support for Linux and Windows.

Features:
- Create and manage WiFi hotspots
- Support multiple authentication methods (WPA-PSK, WPA2, WPA3)
- Save and load hotspot profiles
- Monitor connected clients
- Async operations support
- Comprehensive logging
- Enhanced CLI with rich for better user experience

Author: Your Name
License: MIT
Version: 2.0.0
"""

from __future__ import annotations
import subprocess
import platform
import argparse
from typing import Optional, Literal, Dict, Any, List
from dataclasses import dataclass, asdict
from enum import Enum
import shutil
from pathlib import Path
import json
from datetime import datetime
import asyncio
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.prompt import Prompt, Confirm
from rich.progress import Progress
import sys

# Configure loguru logger with Rich handler
logger.remove()
console = Console()
logger.add(
    sink=sys.stderr,
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | "
           "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - "
           "<level>{message}</level>",
    level="INFO",
    enqueue=True,
    backtrace=True,
    diagnose=True
)
logger.add(
    "hotspot.log",
    rotation="500 MB",
    retention="10 days",
    level="DEBUG",
    format="{time:YYYY-MM-DD at HH:mm:ss} | {level} | {message}"
)

console = Console()


class PlatformType(Enum):
    """
    Supported operating system platforms.
    Used to determine platform-specific implementations.
    """
    LINUX = "linux"
    WINDOWS = "windows"
    MACOS = "darwin"


class AuthType(Enum):
    """
    WiFi authentication types supported by the hotspot.

    Members:
        WPA_PSK: WPA Personal authentication
        WPA2: WPA2 authentication (more secure than WPA)
        WPA3: Latest WPA3 authentication (most secure)
    """
    WPA_PSK = "wpa-psk"
    WPA2 = "wpa2"
    WPA3 = "wpa3"


class EncryptionType(Enum):
    """
    Encryption methods for securing the WiFi connection.

    Members:
        AES: Advanced Encryption Standard (recommended)
        TKIP: Temporal Key Integrity Protocol (legacy)
    """
    AES = "aes"
    TKIP = "tkip"


@dataclass
class HotspotConfig:
    """
    Configuration settings for a WiFi hotspot.

    Attributes:
        name: SSID of the hotspot
        password: Security key for authentication
        authentication: Type of authentication (WPA/WPA2/WPA3)
        encryption: Type of encryption (AES/TKIP)
        channel: WiFi channel number (1-14)
        max_clients: Maximum number of concurrent connections
        band: WiFi frequency band (bg/a/ac)
        hidden: Whether to hide the SSID
    """
    name: str = "MyHotspot"
    password: Optional[str] = None
    authentication: AuthType = AuthType.WPA_PSK
    encryption: EncryptionType = EncryptionType.AES
    channel: int = 11
    max_clients: int = 10
    band: Literal["bg", "a", "ac"] = "bg"
    hidden: bool = False


class HotspotManager:
    """
    Core class for managing WiFi hotspots across different platforms.

    Handles hotspot creation, configuration, monitoring, and profile management.
    Supports both synchronous and asynchronous operations.
    """

    def __init__(self):
        """
        Initialize the HotspotManager with platform detection and config loading.
        Creates necessary directories if they don't exist.
        """
        logger.debug("Initializing HotspotManager")
        self.platform = PlatformType(platform.system().lower())
        self.config_path = Path.home() / ".hotspot"
        self.config_path.mkdir(exist_ok=True)
        self._load_config()

    def _load_config(self) -> None:
        """
        Load saved hotspot configurations from a JSON file.
        If the file does not exist, initialize an empty configuration dictionary.
        """
        logger.debug("Loading hotspot configurations")
        config_file = self.config_path / "config.json"
        if config_file.exists():
            with open(config_file) as f:
                self.saved_configs: Dict[str, Dict[str, Any]] = json.load(f)
            logger.info("Hotspot configurations loaded successfully")
        else:
            self.saved_configs = {}
            logger.info(
                "No existing configurations found, starting with an empty configuration"
            )

    def _save_config(self) -> None:
        """
        Save the current hotspot configurations to a JSON file.
        """
        logger.debug("Saving hotspot configurations")
        with open(self.config_path / "config.json", "w") as f:
            json.dump(self.saved_configs, f, indent=4)
        logger.info("Hotspot configurations saved successfully")

    async def _run_command_async(self, cmd: List[str]) -> tuple[str, str]:
        """
        Run a shell command asynchronously.

        Args:
            cmd: List of command arguments.

        Returns:
            A tuple containing the standard output and standard error of the command.

        Raises:
            Exception: If the command execution fails.
        """
        logger.debug(f"Running command asynchronously: {' '.join(cmd)}")
        try:
            process = await asyncio.create_subprocess_exec(
                *cmd,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            stdout, stderr = await process.communicate()
            stdout_decoded = stdout.decode().strip()
            stderr_decoded = stderr.decode().strip()
            logger.debug(f"Command output: {stdout_decoded}")
            if stderr_decoded:
                logger.error(f"Command error: {stderr_decoded}")
            return stdout_decoded, stderr_decoded
        except Exception as e:
            logger.error(f"Command execution failed: {e}")
            raise

    def _run_command(self, cmd: List[str]) -> str:
        """
        Run a shell command synchronously.

        Args:
            cmd: List of command arguments.

        Returns:
            The standard output of the command.

        Raises:
            subprocess.CalledProcessError: If the command execution fails.
        """
        logger.debug(f"Running command synchronously: {' '.join(cmd)}")
        try:
            result = subprocess.run(
                cmd,
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            logger.debug(f"Command output: {result.stdout.strip()}")
            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed: {e.stderr.strip()}")
            raise

    async def start(self, config: HotspotConfig) -> None:
        """
        Start a WiFi hotspot with the given configuration.

        Args:
            config: HotspotConfig object containing the hotspot settings.

        Raises:
            ValueError: If the password is not provided.
            NotImplementedError: If the platform is not supported.
        """
        logger.info(f"Starting hotspot with name: {config.name}")

        if config.password is None:
            logger.error("Password is required when starting a hotspot")
            raise ValueError("Password is required when starting a hotspot")

        if self.platform == PlatformType.LINUX:
            await self._start_linux(config)
        elif self.platform == PlatformType.WINDOWS:
            await self._start_windows(config)
        else:
            logger.error(f"Platform {self.platform} not supported")
            raise NotImplementedError(
                f"Platform {self.platform} not supported"
            )

        logger.success(f"Hotspot '{config.name}' is now running")

    async def _start_linux(self, config: HotspotConfig) -> None:
        """
        Start a WiFi hotspot on a Linux platform.

        Args:
            config: HotspotConfig object containing the hotspot settings.
        """
        logger.debug(f"Starting Linux hotspot with configuration: {config}")
        cmd = [
            'nmcli', 'dev', 'wifi', 'hotspot',
            'ifname', 'wlan0',
            'ssid', config.name,
            'password', config.password,
            'band', config.band,
            'channel', str(config.channel)
        ]
        await self._run_command_async(cmd)

        # Configure additional parameters if needed
        if config.hidden:
            await self._configure_hidden_hotspot_linux()

    async def _configure_hidden_hotspot_linux(self) -> None:
        """
        Configure the hotspot to hide the SSID on Linux.
        """
        logger.debug("Configuring hidden SSID for Linux hotspot")
        cmd = [
            'nmcli', 'connection', 'modify', 'Hotspot',
            'ssid-hidden', 'yes'
        ]
        await self._run_command_async(cmd)

    async def _start_windows(self, config: HotspotConfig) -> None:
        """
        Start a WiFi hotspot on a Windows platform.

        Args:
            config: HotspotConfig object containing the hotspot settings.
        """
        logger.debug(f"Starting Windows hotspot with configuration: {config}")
        cmd_set = [
            'netsh', 'wlan', 'set', 'hostednetwork',
            'mode=allow',
            f'ssid={config.name}',
            f'key={config.password}',
            f'keyUsage=persistent'
        ]
        await self._run_command_async(cmd_set)

        cmd_start = [
            'netsh', 'wlan', 'start', 'hostednetwork'
        ]
        await self._run_command_async(cmd_start)

    async def stop(self) -> None:
        """
        Stop the running WiFi hotspot.
        """
        logger.info("Stopping hotspot")

        if self.platform == PlatformType.LINUX:
            cmd = ['nmcli', 'connection', 'down', 'Hotspot']
            await self._run_command_async(cmd)
        elif self.platform == PlatformType.WINDOWS:
            cmd = ['netsh', 'wlan', 'stop', 'hostednetwork']
            await self._run_command_async(cmd)
        else:
            logger.error(
                f"Platform {self.platform} not supported for stopping hotspot")
            raise NotImplementedError(
                f"Platform {self.platform} not supported for stopping hotspot"
            )

        logger.success("Hotspot has been stopped")

    async def status(self) -> Dict[str, Any]:
        """
        Get the current status of the WiFi hotspot.

        Returns:
            A dictionary containing the status and details of the hotspot.
        """
        logger.info("Checking hotspot status")
        status_info: Dict[str, Any] = {}

        if self.platform == PlatformType.LINUX:
            stdout, _ = await self._run_command_async(['nmcli', 'connection', 'show', '--active'])
            if 'Hotspot' in stdout:
                status_info['status'] = 'running'
                status_info['details'] = stdout
            else:
                status_info['status'] = 'stopped'
        elif self.platform == PlatformType.WINDOWS:
            stdout, _ = await self._run_command_async(['netsh', 'wlan', 'show', 'hostednetwork'])
            if 'State' in stdout and 'started' in stdout.lower():
                status_info['status'] = 'running'
                status_info['details'] = stdout
            else:
                status_info['status'] = 'stopped'
        else:
            logger.error(
                f"Platform {self.platform} not supported for status check")
            raise NotImplementedError(
                f"Platform {self.platform} not supported for status check"
            )

        logger.debug(f"Hotspot status: {status_info}")
        return status_info

    async def list_clients(self) -> List[Dict[str, Any]]:
        """
        List the clients connected to the WiFi hotspot.

        Returns:
            A list of dictionaries containing client information.
        """
        logger.info("Listing connected clients")
        clients: List[Dict[str, Any]] = []

        if self.platform == PlatformType.LINUX:
            stdout, _ = await self._run_command_async(['iw', 'dev', 'wlan0', 'station', 'dump'])
            logger.debug(f"Connected clients (Linux): {stdout}")
            clients = self._parse_linux_clients(stdout)
        elif self.platform == PlatformType.WINDOWS:
            stdout, _ = await self._run_command_async(['netsh', 'wlan', 'show', 'hostednetwork'])
            logger.debug(f"Connected clients (Windows): {stdout}")
            clients = self._parse_windows_clients(stdout)
        else:
            logger.error(
                f"Platform {self.platform} not supported for listing clients")
            raise NotImplementedError(
                f"Platform {self.platform} not supported for listing clients"
            )

        return clients

    def _parse_linux_clients(self, raw_output: str) -> List[Dict[str, Any]]:
        """
        Parse the output from the Linux command to list clients.

        Args:
            raw_output: Raw string output from the command.

        Returns:
            A list of dictionaries with client information.
        """
        clients = []
        current_client = {}
        for line in raw_output.split('\n'):
            if line.startswith('Station'):
                if current_client:
                    clients.append(current_client)
                    current_client = {}
                current_client['MAC Address'] = line.split()[1]
            elif ':' in line:
                key, value = line.split(':', 1)
                current_client[key.strip()] = value.strip()
        if current_client:
            clients.append(current_client)
        logger.debug(f"Parsed Linux clients: {clients}")
        return clients

    def _parse_windows_clients(self, raw_output: str) -> List[Dict[str, Any]]:
        """
        Parse the output from the Windows command to list clients.

        Args:
            raw_output: Raw string output from the command.

        Returns:
            A list of dictionaries with client information.
        """
        clients = []
        lines = raw_output.splitlines()
        client_section = False
        for line in lines:
            if 'SSID' in line and 'BSSID' not in line:
                client_section = False
            if 'Client' in line and 'SSID' not in line:
                client_section = True
                continue
            if client_section:
                if line.strip():
                    parts = line.split(':', 1)
                    if len(parts) == 2:
                        key, value = parts
                        clients.append({key.strip(): value.strip()})
        logger.debug(f"Parsed Windows clients: {clients}")
        return clients

    def save_profile(self, name: str, config: HotspotConfig) -> None:
        """
        Save a hotspot configuration profile.

        Args:
            name: Name of the profile.
            config: HotspotConfig object containing the hotspot settings.
        """
        logger.info(f"Saving profile: {name}")
        self.saved_configs[name] = asdict(config)
        self._save_config()
        logger.info(f"Profile '{name}' has been saved")

    def load_profile(self, name: str) -> HotspotConfig:
        """
        Load a hotspot configuration profile.

        Args:
            name: Name of the profile.

        Returns:
            A HotspotConfig object containing the loaded settings.

        Raises:
            ValueError: If the profile is not found.
        """
        logger.info(f"Loading profile: {name}")
        if name not in self.saved_configs:
            logger.error(f"Profile '{name}' not found")
            raise ValueError(f"Profile '{name}' not found")

        config_dict = self.saved_configs[name]
        logger.debug(
            f"Profile '{name}' loaded with configuration: {config_dict}")
        return HotspotConfig(**config_dict)


async def main() -> None:
    """
    Main function to parse command-line arguments and perform actions.
    """
    parser = argparse.ArgumentParser(
        description='Advanced WiFi Hotspot Manager with Enhanced Features'
    )
    subparsers = parser.add_subparsers(
        dest='command', help='Available commands')

    # Start command
    parser_start = subparsers.add_parser('start', help='Start a WiFi hotspot')
    parser_start.add_argument(
        '--name', type=str, default='MyHotspot', help='Hotspot name (SSID)')
    parser_start.add_argument('--password', type=str,
                              required=True, help='Hotspot password')
    parser_start.add_argument('--authentication', type=str, choices=[t.value for t in AuthType], default='wpa-psk',
                              help='Authentication type')
    parser_start.add_argument('--encryption', type=str, choices=[t.value for t in EncryptionType], default='aes',
                              help='Encryption type')
    parser_start.add_argument('--channel', type=int,
                              default=11, help='WiFi channel number (1-14)')
    parser_start.add_argument(
        '--max-clients', type=int, default=10, help='Maximum number of clients')
    parser_start.add_argument(
        '--band', type=str, choices=['bg', 'a', 'ac'], default='bg', help='WiFi frequency band')
    parser_start.add_argument(
        '--hidden', action='store_true', help='Hide the SSID')

    # Stop command
    parser_stop = subparsers.add_parser('stop', help='Stop the WiFi hotspot')

    # Status command
    parser_status = subparsers.add_parser(
        'status', help='Check the status of the WiFi hotspot')

    # List clients command
    parser_list = subparsers.add_parser(
        'list-clients', help='List connected clients')

    # Save profile command
    parser_save = subparsers.add_parser(
        'save-profile', help='Save a hotspot profile')
    parser_save.add_argument('--profile', type=str,
                             required=True, help='Profile name to save')
    parser_save.add_argument(
        '--name', type=str, default='MyHotspot', help='Hotspot name (SSID)')
    parser_save.add_argument('--authentication', type=str, choices=[t.value for t in AuthType], default='wpa-psk',
                             help='Authentication type')
    parser_save.add_argument('--encryption', type=str, choices=[t.value for t in EncryptionType], default='aes',
                             help='Encryption type')
    parser_save.add_argument('--channel', type=int,
                             default=11, help='WiFi channel number (1-14)')
    parser_save.add_argument('--max-clients', type=int,
                             default=10, help='Maximum number of clients')
    parser_save.add_argument(
        '--band', type=str, choices=['bg', 'a', 'ac'], default='bg', help='WiFi frequency band')
    parser_save.add_argument(
        '--hidden', action='store_true', help='Hide the SSID')

    # Load profile command
    parser_load = subparsers.add_parser(
        'load-profile', help='Load a hotspot profile and start hotspot')
    parser_load.add_argument('--profile', type=str,
                             required=True, help='Profile name to load')

    args = parser.parse_args()

    manager = HotspotManager()

    try:
        if args.command == 'start':
            config = HotspotConfig(
                name=args.name,
                password=args.password,
                authentication=AuthType(args.authentication),
                encryption=EncryptionType(args.encryption),
                channel=args.channel,
                max_clients=args.max_clients,
                band=args.band,
                hidden=args.hidden
            )
            await manager.start(config)

        elif args.command == 'stop':
            await manager.stop()

        elif args.command == 'status':
            status = await manager.status()
            table = Table(title="Hotspot Status")
            table.add_column("Key", style="cyan", no_wrap=True)
            table.add_column("Value", style="magenta")
            for key, value in status.items():
                table.add_row(key, value)
            console.print(table)

        elif args.command == 'list-clients':
            clients = await manager.list_clients()
            if clients:
                table = Table(title="Connected Clients")
                for client in clients:
                    for key, value in client.items():
                        table.add_column(key, style="cyan", no_wrap=True)
                for client in clients:
                    row = [str(value) for value in client.values()]
                    table.add_row(*row)
                console.print(table)
            else:
                console.print("[yellow]No clients are connected.[/yellow]")

        elif args.command == 'save-profile':
            config = HotspotConfig(
                name=args.name,
                authentication=AuthType(args.authentication),
                encryption=EncryptionType(args.encryption),
                channel=args.channel,
                max_clients=args.max_clients,
                band=args.band,
                hidden=args.hidden
            )
            manager.save_profile(args.profile, config)

        elif args.command == 'load-profile':
            config = manager.load_profile(args.profile)
            await manager.start(config)

        else:
            parser.print_help()
            sys.exit(1)

    except Exception as e:
        logger.error(f"Error: {str(e)}")
        console.print(f"[red]Error: {str(e)}[/red]")
        sys.exit(1)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except Exception as e:
        logger.critical(f"Unhandled exception: {e}")
        console.print(f"[red]Unhandled exception: {e}[/red]")
        sys.exit(1)
