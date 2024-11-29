# python
import subprocess
import sys
import platform
import shutil
from loguru import logger
from pathlib import Path
from typing import Dict, Callable

from rich.console import Console
from rich.table import Table
from rich.markdown import Markdown

console = Console()

# Configure loguru logger
logger.remove()
logger.add(
    sys.stderr,
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | "
           "<level>{level: <8}</level> | "
           "<cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - "
           "<level>{message}</level>",
    level="INFO"
)
logger.add("nginx_manager.log", rotation="500 MB", retention="10 days")

# Define default Nginx paths based on the operating system
DEFAULT_NGINX_PATH = Path(
    "/etc/nginx") if platform.system() != "Windows" else Path("C:/nginx")
DEFAULT_NGINX_CONF = DEFAULT_NGINX_PATH / \
    "nginx.conf" if platform.system() != "Windows" else DEFAULT_NGINX_PATH / \
    "conf/nginx.conf"
DEFAULT_NGINX_BINARY = Path(
    "/usr/sbin/nginx") if platform.system() != "Windows" else DEFAULT_NGINX_PATH / "nginx.exe"
DEFAULT_BACKUP_PATH = DEFAULT_NGINX_PATH / \
    "backup" if platform.system() != "Windows" else DEFAULT_NGINX_PATH / "backup"


class NginxManager:
    """
    A class to manage Nginx operations such as start, stop, reload, etc.
    """

    def __init__(
        self,
        nginx_path: Path = DEFAULT_NGINX_PATH,
        nginx_conf: Path = DEFAULT_NGINX_CONF,
        nginx_binary: Path = DEFAULT_NGINX_BINARY,
        backup_path: Path = DEFAULT_BACKUP_PATH
    ):
        self.nginx_path = nginx_path
        self.nginx_conf = nginx_conf
        self.nginx_binary = nginx_binary
        self.backup_path = backup_path
        self._ensure_paths()

    def _ensure_paths(self):
        """Ensure that the necessary paths exist."""
        logger.debug("Ensuring Nginx paths exist")
        if not self.nginx_binary.exists():
            logger.error(f"Nginx binary not found at {self.nginx_binary}")
            sys.exit(1)
        if not self.nginx_conf.exists():
            logger.error(
                f"Nginx configuration file not found at {self.nginx_conf}")
            sys.exit(1)
        if not self.backup_path.exists():
            self.backup_path.mkdir(parents=True, exist_ok=True)
            logger.debug(f"Created backup directory at {self.backup_path}")

    def start(self):
        """Start Nginx server."""
        logger.info("Starting Nginx")
        try:
            subprocess.run([str(self.nginx_binary)], check=True)
            logger.success("Nginx has been started")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to start Nginx: {e}")

    def stop(self):
        """Stop Nginx server."""
        logger.info("Stopping Nginx")
        try:
            subprocess.run([str(self.nginx_binary), '-s', 'stop'], check=True)
            logger.success("Nginx has been stopped")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to stop Nginx: {e}")

    def reload(self):
        """Reload Nginx configuration."""
        logger.info("Reloading Nginx configuration")
        try:
            subprocess.run(
                [str(self.nginx_binary), '-s', 'reload'], check=True)
            logger.success("Nginx configuration has been reloaded")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to reload Nginx: {e}")

    def restart(self):
        """Restart Nginx server."""
        logger.info("Restarting Nginx")
        self.stop()
        self.start()

    def check_config(self):
        """Check Nginx configuration syntax."""
        logger.info("Checking Nginx configuration syntax")
        try:
            subprocess.run(
                [str(self.nginx_binary), '-t', '-c', str(self.nginx_conf)],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            logger.success("Nginx configuration syntax is correct")
        except subprocess.CalledProcessError as e:
            logger.error("Nginx configuration syntax is incorrect")
            console.print("[bold red]Error details:[/bold red]")
            console.print(e.stderr.decode())

    def show_status(self):
        """Show Nginx running status."""
        logger.info("Checking Nginx status")
        try:
            result = subprocess.run(
                ["pgrep", "nginx"], check=True, stdout=subprocess.PIPE
            )
            if result.stdout:
                logger.success("Nginx is running")
            else:
                logger.info("Nginx is not running")
        except subprocess.CalledProcessError:
            logger.info("Nginx is not running")

    def show_version(self):
        """Show Nginx version information."""
        logger.info("Showing Nginx version")
        try:
            result = subprocess.run(
                [str(self.nginx_binary), '-v'], stderr=subprocess.PIPE, check=True
            )
            version_info = result.stderr.decode().strip()
            console.print(f"[bold green]{version_info}[/bold green]")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to get Nginx version: {e}")

    def backup_config(self):
        """Backup Nginx configuration file."""
        logger.info("Backing up Nginx configuration file")
        backup_file = self.backup_path / "nginx.conf.bak"
        try:
            shutil.copy(self.nginx_conf, backup_file)
            logger.success(f"Nginx configuration backed up to {backup_file}")
        except IOError as e:
            logger.error(f"Failed to backup configuration: {e}")

    def restore_config(self):
        """Restore Nginx configuration file from backup."""
        logger.info("Restoring Nginx configuration from backup")
        backup_file = self.backup_path / "nginx.conf.bak"
        if backup_file.exists():
            try:
                shutil.copy(backup_file, self.nginx_conf)
                logger.success(
                    "Nginx configuration has been restored from backup")
            except IOError as e:
                logger.error(f"Failed to restore configuration: {e}")
        else:
            logger.error("Backup file not found")

    def test_config(self):
        """Test Nginx configuration without reloading."""
        logger.info("Testing Nginx configuration")
        try:
            subprocess.run(
                [str(self.nginx_binary), '-t', '-c', str(self.nginx_conf)],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            logger.success("Nginx configuration test passed")
        except subprocess.CalledProcessError as e:
            logger.error("Nginx configuration test failed")
            console.print("[bold red]Error details:[/bold red]")
            console.print(e.stderr.decode())

    def view_logs(self, log_type: str = 'access'):
        """View Nginx logs.

        Args:
            log_type (str): The type of log to view ('access' or 'error').
        """
        log_file = self.nginx_path / f"logs/{log_type}.log"
        logger.info(f"Viewing Nginx {log_type} log")
        if log_file.exists():
            with log_file.open() as f:
                for line in f:
                    console.print(line.strip())
        else:
            logger.error(
                f"{log_type.capitalize()} log file not found at {log_file}")

    def clear_logs(self, log_type: str = 'access'):
        """Clear Nginx logs.

        Args:
            log_type (str): The type of log to clear ('access' or 'error').
        """
        log_file = self.nginx_path / f"logs/{log_type}.log"
        logger.info(f"Clearing Nginx {log_type} log")
        if log_file.exists():
            with log_file.open('w') as f:
                f.truncate()
            logger.success(f"{log_type.capitalize()} log has been cleared")
        else:
            logger.error(
                f"{log_type.capitalize()} log file not found at {log_file}")

    def list_sites(self):
        """List available and enabled sites."""
        sites_available = self.nginx_path / "sites-available"
        sites_enabled = self.nginx_path / "sites-enabled"

        logger.info("Listing Nginx sites")
        table = Table(title="Nginx Sites")
        table.add_column("Site", style="cyan")
        table.add_column("Status", style="green")

        available_sites = {site.name for site in sites_available.glob('*')}
        enabled_sites = {site.name for site in sites_enabled.glob('*')}

        all_sites = available_sites.union(enabled_sites)

        for site in all_sites:
            status = "Enabled" if site in enabled_sites else "Disabled"
            table.add_row(site, status)

        console.print(table)

    # Additional methods can be added here...


def show_help():
    """Show help message."""
    help_text = """
    Nginx Manager - A tool to manage Nginx server operations.

    Usage:
      python nginx_manager.py [command]

    Commands:
      start          Start Nginx server
      stop           Stop Nginx server
      reload         Reload Nginx configuration
      restart        Restart Nginx server
      test           Test Nginx configuration
      check          Check Nginx configuration syntax
      status         Show Nginx running status
      version        Show Nginx version information
      backup         Backup Nginx configuration file
      restore        Restore Nginx configuration from backup
      view_logs      View Nginx logs (access or error)
      clear_logs     Clear Nginx logs (access or error)
      list_sites     List available and enabled sites
      help           Show this help message

    Examples:
      python nginx_manager.py start
      python nginx_manager.py view_logs access
    """
    console.print(Markdown(help_text))
    logger.info("Displayed help message")


def main():
    manager = NginxManager()

    commands: Dict[str, Callable] = {
        "start": manager.start,
        "stop": manager.stop,
        "reload": manager.reload,
        "restart": manager.restart,
        "test": manager.test_config,
        "check": manager.check_config,
        "status": manager.show_status,
        "version": manager.show_version,
        "backup": manager.backup_config,
        "restore": manager.restore_config,
        "view_logs": manager.view_logs,
        "clear_logs": manager.clear_logs,
        "list_sites": manager.list_sites,
        "help": show_help
    }

    if len(sys.argv) < 2:
        show_help()
        sys.exit(1)

    command = sys.argv[1]

    if command in commands:
        try:
            if command in ["view_logs", "clear_logs"]:
                # Optional argument for log type
                log_type = sys.argv[2] if len(sys.argv) > 2 else 'access'
                commands[command](log_type)
            else:
                commands[command]()
        except Exception as e:
            logger.exception(
                f"An error occurred while executing '{command}': {e}")
    else:
        logger.error(f"Invalid command '{command}'")
        show_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
