import subprocess
import os
import sys
import platform
import shutil
from loguru import logger
from pathlib import Path

# Define Nginx paths
NGINX_PATH = "/etc/nginx" if platform.system() != "Windows" else "C:\\nginx"
NGINX_CONF = f"{NGINX_PATH}/nginx.conf" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\conf\\nginx.conf"
NGINX_BINARY = "/usr/sbin/nginx" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\nginx.exe"
BACKUP_PATH = f"{NGINX_PATH}/backup" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\backup"

# Configure loguru logger
logger.remove()
logger.add(
    sys.stderr,
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
    level="INFO"
)
logger.add("nginx_manager.log", rotation="500 MB", retention="10 days")


def install_nginx():
    """Install Nginx if not already installed"""
    logger.info("Checking if Nginx is installed")
    try:
        result = subprocess.run([NGINX_BINARY, "-v"],
                                stderr=subprocess.PIPE, check=True)
        logger.info("Nginx is already installed")
    except subprocess.CalledProcessError:
        logger.info("Nginx is not installed, installing now")
        if platform.system() == "Linux":
            if os.path.isfile("/etc/debian_version"):
                subprocess.run(
                    "sudo apt-get update && sudo apt-get install nginx -y", shell=True, check=True)
            elif os.path.isfile("/etc/redhat-release"):
                subprocess.run(
                    "sudo yum update && sudo yum install nginx -y", shell=True, check=True)
            else:
                logger.error(
                    "Unsupported Linux distribution. Please install Nginx manually.")
                sys.exit(1)
        elif platform.system() == "Windows":
            logger.error("Please install Nginx manually on Windows.")
            sys.exit(1)
        else:
            logger.error(
                "Unsupported platform. Please install Nginx manually.")
            sys.exit(1)


def start_nginx():
    """Start Nginx"""
    logger.info("Starting Nginx")
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY], check=True)
        logger.success("Nginx has been started")
    else:
        logger.error("Nginx binary not found")


def stop_nginx():
    """Stop Nginx"""
    logger.info("Stopping Nginx")
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY, '-s', 'stop'], check=True)
        logger.success("Nginx has been stopped")
    else:
        logger.error("Nginx binary not found")


def reload_nginx():
    """Reload Nginx configuration"""
    logger.info("Reloading Nginx configuration")
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY, '-s', 'reload'], check=True)
        logger.success("Nginx configuration has been reloaded")
    else:
        logger.error("Nginx binary not found")


def restart_nginx():
    """Restart Nginx"""
    logger.info("Restarting Nginx")
    stop_nginx()
    start_nginx()


def check_config():
    """Check Nginx configuration syntax"""
    logger.info("Checking Nginx configuration syntax")
    if os.path.isfile(NGINX_CONF):
        result = subprocess.run(
            [NGINX_BINARY, '-t', '-c', NGINX_CONF], check=True)
        if result.returncode == 0:
            logger.success("Nginx configuration syntax is correct")
        else:
            logger.error("Nginx configuration syntax is incorrect")
    else:
        logger.error("Nginx configuration file not found")


def show_status():
    """Show Nginx status"""
    logger.info("Checking Nginx status")
    result = subprocess.run("pgrep nginx", shell=True, stdout=subprocess.PIPE)
    if result.stdout:
        logger.success("Nginx is running")
    else:
        logger.error("Nginx is not running")


def show_version():
    """Show Nginx version"""
    logger.info("Showing Nginx version")
    result = subprocess.run([NGINX_BINARY, '-v'],
                            stderr=subprocess.PIPE, check=True)
    logger.info(result.stderr.decode())


def backup_config():
    """Backup Nginx configuration file"""
    logger.info("Backing up Nginx configuration file")
    if not os.path.exists(BACKUP_PATH):
        os.makedirs(BACKUP_PATH)
    backup_file = os.path.join(BACKUP_PATH, "nginx.conf.bak")
    shutil.copy(NGINX_CONF, backup_file)
    logger.success(
        f"Nginx configuration file has been backed up to {backup_file}")


def restore_config():
    """Restore Nginx configuration file"""
    logger.info("Restoring Nginx configuration file from backup")
    backup_file = os.path.join(BACKUP_PATH, "nginx.conf.bak")
    if os.path.isfile(backup_file):
        shutil.copy(backup_file, NGINX_CONF)
        logger.success(
            "Nginx configuration file has been restored from backup")
    else:
        logger.error("Backup file not found")


def show_help():
    """Show help message"""
    help_message = """
    Usage: python nginx_manager.py [start|stop|reload|restart|check|status|version|backup|restore|help]
      start    Start Nginx
      stop     Stop Nginx
      reload   Reload Nginx configuration
      restart  Restart Nginx
      check    Check Nginx configuration syntax
      status   Show Nginx status
      version  Show Nginx version
      backup   Backup Nginx configuration file
      restore  Restore Nginx configuration file
      help     Show help message
    """
    print(help_message)
    logger.info("Displayed help message")


def main():
    if len(sys.argv) < 2:
        show_help()
        sys.exit(1)

    command = sys.argv[1]

    # Check if Nginx is installed
    install_nginx()

    commands = {
        "start": start_nginx,
        "stop": stop_nginx,
        "reload": reload_nginx,
        "restart": restart_nginx,
        "check": check_config,
        "status": show_status,
        "version": show_version,
        "backup": backup_config,
        "restore": restore_config,
        "help": show_help
    }

    if command in commands:
        commands[command]()
    else:
        logger.error("Invalid command")
        show_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
