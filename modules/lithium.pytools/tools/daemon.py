#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Daemon Process Manager

A robust daemon manager for monitoring and managing target processes.
Supports starting, stopping, and monitoring processes with resource usage checks.

Features:
- Start and stop daemon processes
- Monitor CPU and memory usage
- Automatic process restarts on threshold breaches
- Comprehensive logging with loguru
- Enhanced CLI output with rich
- Detailed configuration with dataclasses

Author: Your Name
License: MIT
Version: 2.0.0
"""

import argparse
import asyncio
import json
import os
import platform
import signal
import subprocess
import sys
import time
from dataclasses import dataclass, asdict
from datetime import datetime
from enum import Enum
from pathlib import Path
from typing import Optional, Literal, Dict, Any, List

import psutil
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.prompt import Confirm
from rich.panel import Panel
from rich.progress import Progress, SpinnerColumn, BarColumn, TextColumn
from rich.logging import RichHandler

# Configure Loguru logger with rich handler
logger.remove()  # Remove the default logger
console = Console()
logger.add(
    RichHandler(console=console),
    rotation="10 MB",
    retention="10 days",
    level="INFO",
    format="{time:YYYY-MM-DD at HH:mm:ss} | {level} | {message}"
)

# Default configuration values
DEFAULT_CONFIG = {
    "process_name": "python",
    "script_path": Path("target_script.py"),
    "restart_interval": 5,
    "cpu_threshold": 80.0,
    "memory_threshold": 500.0,
    "max_restarts": 3,
    "monitor_interval": 5
}

# Daemon configuration parameters using dataclass


@dataclass
class DaemonConfig:
    process_name: str = "python"             # Name of the process to monitor
    script_path: Path = Path("target_script.py")  # Path to the target script
    restart_interval: int = 5                # Restart interval in seconds
    cpu_threshold: float = 80.0              # CPU usage threshold in percentage
    memory_threshold: float = 500.0          # Memory usage threshold in MB
    max_restarts: int = 3                     # Maximum number of restarts
    monitor_interval: int = 5                 # Monitoring interval in seconds


PID_FILE = "/tmp/daemon.pid"


class Platform(Enum):
    """
    Supported operating system platforms.
    Used to determine platform-specific implementations.
    """
    LINUX = "linux"
    WINDOWS = "windows"
    MACOS = "darwin"


class DaemonProcess:
    """
    Core class for managing the daemon process.

    Handles starting, monitoring, restarting, and stopping the target process.
    """

    def __init__(self, config: DaemonConfig):
        """
        Initialize the DaemonProcess with the given configuration.

        Args:
            config (DaemonConfig): Configuration settings for the daemon.
        """
        self.config = config
        self.restart_count = 0
        self.process: Optional[subprocess.Popen] = None
        self.platform = Platform(platform.system().lower())
        logger.debug(f"DaemonProcess initialized with config: {self.config}")

        if not self.config.script_path.is_file():
            logger.critical(
                f"Target script does not exist: {self.config.script_path}")
            raise FileNotFoundError(
                f"Target script does not exist: {self.config.script_path}")

    async def start_target_process(self) -> None:
        """
        Start the target process asynchronously.
        """
        try:
            self.process = subprocess.Popen(
                [sys.executable, str(self.config.script_path)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            logger.info(f"Target process started, PID: {self.process.pid}")
        except Exception as e:
            logger.exception(f"Failed to start target process: {e}")
            raise

    def is_process_running(self) -> bool:
        """
        Check if the target process is running.

        Returns:
            bool: True if running, False otherwise.
        """
        if self.process is None:
            logger.debug("Target process instance is None.")
            return False

        running = self.process.poll() is None
        logger.debug(
            f"Process status: {'Running' if running else 'Stopped'} (PID: {self.process.pid})")
        return running

    async def monitor_process_health(self) -> None:
        """
        Monitor the CPU and memory usage of the process. Restart if thresholds are exceeded.
        """
        try:
            proc = psutil.Process(self.process.pid)
            cpu_usage = proc.cpu_percent(interval=1)
            memory_usage = proc.memory_info().rss / (1024 * 1024)  # Convert to MB
            logger.info(
                f"Process PID: {proc.pid}, CPU: {cpu_usage}%, Memory: {memory_usage:.2f}MB")

            if cpu_usage > self.config.cpu_threshold:
                logger.warning(
                    f"CPU usage exceeded threshold ({cpu_usage}% > {self.config.cpu_threshold}%), restarting process...")
                await self.restart_process()

            if memory_usage > self.config.memory_threshold:
                logger.warning(
                    f"Memory usage exceeded threshold ({memory_usage}MB > {self.config.memory_threshold}MB), restarting process...")
                await self.restart_process()

        except psutil.NoSuchProcess:
            logger.warning(
                "Process does not exist, may have crashed. Will restart...")
            await self.restart_process()
        except psutil.AccessDenied:
            logger.error("Access denied when accessing process information.")
        except Exception as e:
            logger.exception(
                f"Unknown error occurred while monitoring process health: {e}")

    async def restart_process(self) -> None:
        """
        Restart the target process, track restart counts, and check against maximum restarts.
        """
        if self.restart_count < self.config.max_restarts:
            try:
                if self.process and self.is_process_running():
                    self.process.terminate()
                    try:
                        self.process.wait(timeout=5)
                        logger.info(
                            f"Process PID: {self.process.pid} terminated.")
                    except subprocess.TimeoutExpired:
                        self.process.kill()
                        self.process.wait()
                        logger.warning(
                            f"Process PID: {self.process.pid} force killed.")

                self.restart_count += 1
                logger.info(
                    f"Restarting target process (Restart count: {self.restart_count})")
                await asyncio.sleep(self.config.restart_interval)
                await self.start_target_process()
            except Exception as e:
                logger.exception(f"Failed to restart process: {e}")
        else:
            logger.error("Maximum restart count reached, stopping daemon.")
            self.cleanup()
            sys.exit("Daemon terminated: exceeded maximum restart count.")

    async def monitor_loop(self) -> None:
        """
        Main loop of the daemon for continuous monitoring of the target process.
        """
        await self.start_target_process()

        try:
            while True:
                if not self.is_process_running():
                    logger.warning(
                        "Target process is not running, restarting...")
                    await self.restart_process()
                else:
                    await self.monitor_process_health()

                await asyncio.sleep(self.config.monitor_interval)
        except KeyboardInterrupt:
            logger.info("Daemon received interrupt signal, exiting...")
        except Exception as e:
            logger.exception(f"Error occurred while running daemon: {e}")
        finally:
            self.cleanup()

    def cleanup(self) -> None:
        """
        Clean up resources and terminate the target process.
        """
        if self.process and self.is_process_running():
            try:
                self.process.terminate()
                self.process.wait(timeout=5)
                logger.info("Target process terminated.")
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
                logger.warning("Target process force killed.")
            except Exception as e:
                logger.exception(
                    f"Error occurred while terminating process: {e}")

        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
            logger.debug("PID file removed.")


def write_pid() -> None:
    """
    Write the daemon's PID to the PID file.
    """
    pid = os.getpid()
    with open(PID_FILE, 'w', encoding='utf-8') as f:
        f.write(str(pid))
    logger.debug(f"PID {pid} written to {PID_FILE}")


def read_pid() -> Optional[int]:
    """
    Read the PID from the PID file.

    Returns:
        Optional[int]: PID as an integer or None if failed.
    """
    try:
        with open(PID_FILE, 'r', encoding='utf-8') as f:
            pid = int(f.read().strip())
        return pid
    except Exception as e:
        logger.error(f"Failed to read PID file: {e}")
        return None


def is_daemon_running(config: DaemonConfig) -> bool:
    """
    Check if the daemon is currently running.

    Args:
        config (DaemonConfig): Configuration dictionary.

    Returns:
        bool: True if running, False otherwise.
    """
    pid = read_pid()
    if pid and psutil.pid_exists(pid):
        try:
            proc = psutil.Process(pid)
            if proc.name() == config.process_name:
                return True
        except psutil.NoSuchProcess:
            return False
    return False


def stop_daemon() -> None:
    """
    Stop the daemon process.
    """
    pid = read_pid()
    if not pid:
        logger.error(
            "PID file not found or PID is invalid. Daemon may not be running.")
        console.print("[red]Daemon is not running.[/red]")
        return

    try:
        proc = psutil.Process(pid)
        logger.info(f"Sending SIGTERM signal to daemon PID: {pid}")
        proc.send_signal(signal.SIGTERM)
        proc.wait(timeout=10)
        logger.info("Daemon has been stopped.")
        console.print("[green]Daemon has been stopped.[/green]")
    except psutil.NoSuchProcess:
        logger.error("Specified daemon process does not exist.")
        console.print("[red]Daemon does not exist.[/red]")
    except psutil.TimeoutExpired:
        logger.warning("Daemon did not respond, sending SIGKILL signal.")
        proc.kill()
        console.print("[yellow]Daemon has been forcefully stopped.[/yellow]")
    except Exception as e:
        logger.exception(f"Error occurred while stopping daemon: {e}")
        console.print(f"[red]Error occurred while stopping daemon: {e}[/red]")
    finally:
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
            logger.debug("PID file removed.")


def start_daemon(config: DaemonConfig) -> None:
    """
    Start the daemon process.

    Args:
        config (DaemonConfig): Configuration settings.
    """
    if is_daemon_running(config):
        logger.error("Daemon is already running.")
        console.print("[red]Daemon is already running.[/red]")
        sys.exit(1)

    # Fork the first child
    try:
        pid = os.fork()
        if pid > 0:
            # Parent process exits
            sys.exit(0)
    except OSError as e:
        logger.exception(f"First fork failed: {e}")
        sys.exit(1)

    # Create a new session
    os.setsid()

    # Fork the second child
    try:
        pid = os.fork()
        if pid > 0:
            # Second parent process exits
            sys.exit(0)
    except OSError as e:
        logger.exception(f"Second fork failed: {e}")
        sys.exit(1)

    # Redirect standard file descriptors to /dev/null
    sys.stdout.flush()
    sys.stderr.flush()
    with open('/dev/null', 'rb', 0) as f:
        os.dup2(f.fileno(), sys.stdin.fileno())
    with open('/dev/null', 'ab', 0) as f:
        os.dup2(f.fileno(), sys.stdout.fileno())
        os.dup2(f.fileno(), sys.stderr.fileno())

    # Write PID file
    write_pid()

    # Instantiate daemon process and start monitoring
    daemon = DaemonProcess(config)

    # Set up signal handlers
    def handle_signal(signum, frame):
        logger.info(f"Received signal {signum}, preparing to exit daemon...")
        daemon.cleanup()
        sys.exit(0)

    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    # Start monitoring loop
    asyncio.run(daemon.monitor_loop())


def status_daemon(config: DaemonConfig) -> None:
    """
    Check the status of the daemon process.

    Args:
        config (DaemonConfig): Configuration settings.
    """
    if is_daemon_running(config):
        pid = read_pid()
        logger.info(f"Daemon is running, PID: {pid}")
        console.print(f"[green]Daemon is running, PID: {pid}[/green]")
    else:
        logger.info("Daemon is not running.")
        console.print("[red]Daemon is not running.[/red]")


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(description="Daemon Management Tool")
    subparsers = parser.add_subparsers(dest="command", help="Sub-commands")

    # Start command
    start_parser = subparsers.add_parser("start", help="Start daemon")
    start_parser.add_argument(
        "--process_name", type=str, default=DEFAULT_CONFIG["process_name"],
        help="Name of the process to monitor"
    )
    start_parser.add_argument(
        "--script_path", type=Path, default=DEFAULT_CONFIG["script_path"],
        help="Path to the target script"
    )
    start_parser.add_argument(
        "--restart_interval", type=int, default=DEFAULT_CONFIG["restart_interval"],
        help="Restart interval in seconds"
    )
    start_parser.add_argument(
        "--cpu_threshold", type=float, default=DEFAULT_CONFIG["cpu_threshold"],
        help="CPU usage threshold (%)"
    )
    start_parser.add_argument(
        "--memory_threshold", type=float, default=DEFAULT_CONFIG["memory_threshold"],
        help="Memory usage threshold (MB)"
    )
    start_parser.add_argument(
        "--max_restarts", type=int, default=DEFAULT_CONFIG["max_restarts"],
        help="Maximum number of restarts"
    )
    start_parser.add_argument(
        "--monitor_interval", type=int, default=DEFAULT_CONFIG["monitor_interval"],
        help="Monitoring interval in seconds"
    )

    # Stop command
    subparsers.add_parser("stop", help="Stop daemon")

    # Status command
    subparsers.add_parser("status", help="Check daemon status")

    return parser.parse_args()


async def main():
    """
    Main function to parse command-line arguments and perform actions.
    """
    args = parse_arguments()
    config = DaemonConfig(
        process_name=args.process_name if hasattr(
            args, 'process_name') else DEFAULT_CONFIG["process_name"],
        script_path=args.script_path if hasattr(
            args, 'script_path') else DEFAULT_CONFIG["script_path"],
        restart_interval=args.restart_interval if hasattr(
            args, 'restart_interval') else DEFAULT_CONFIG["restart_interval"],
        cpu_threshold=args.cpu_threshold if hasattr(
            args, 'cpu_threshold') else DEFAULT_CONFIG["cpu_threshold"],
        memory_threshold=args.memory_threshold if hasattr(
            args, 'memory_threshold') else DEFAULT_CONFIG["memory_threshold"],
        max_restarts=args.max_restarts if hasattr(
            args, 'max_restarts') else DEFAULT_CONFIG["max_restarts"],
        monitor_interval=args.monitor_interval if hasattr(
            args, 'monitor_interval') else DEFAULT_CONFIG["monitor_interval"]
    )

    if args.command == "start":
        try:
            start_daemon(config)
        except Exception as e:
            logger.critical(f"Failed to start daemon: {e}")
            console.print(f"[red]Failed to start daemon: {e}[/red]")
            sys.exit(1)
    elif args.command == "stop":
        stop_daemon()
    elif args.command == "status":
        status_daemon(config)
    else:
        console.print("[yellow]Use -h for help.[/yellow]")
        sys.exit(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except Exception as e:
        logger.exception(f"Unhandled exception: {e}")
        sys.exit(1)
