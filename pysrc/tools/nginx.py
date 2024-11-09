import subprocess
import os
import sys
import platform
import shutil

# Define Nginx paths
NGINX_PATH = "/etc/nginx" if platform.system() != "Windows" else "C:\\nginx"
NGINX_CONF = f"{NGINX_PATH}/nginx.conf" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\conf\\nginx.conf"
NGINX_BINARY = "/usr/sbin/nginx" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\nginx.exe"
BACKUP_PATH = f"{NGINX_PATH}/backup" if platform.system(
) != "Windows" else f"{NGINX_PATH}\\backup"

# Define output colors
GREEN = '\033[0;32m' if platform.system() != "Windows" else ""
RED = '\033[0;31m' if platform.system() != "Windows" else ""
NC = '\033[0m' if platform.system() != "Windows" else ""


def install_nginx():
    """Install Nginx if not already installed"""
    if platform.system() == "Linux":
        result = subprocess.run("nginx -v", shell=True,
                                stderr=subprocess.PIPE, check=True)
        if result.returncode != 0:
            print("Installing Nginx...")
            if os.path.isfile("/etc/debian_version"):
                subprocess.run(
                    "sudo apt-get update && sudo apt-get install nginx -y", shell=True, check=True)
            elif os.path.isfile("/etc/redhat-release"):
                subprocess.run(
                    "sudo yum update && sudo yum install nginx -y", shell=True, check=True)
            else:
                print(
                    f"{RED}Unsupported platform. Please install Nginx manually.{NC}")
                sys.exit(1)


def start_nginx():
    """Start Nginx"""
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY], check=True)
        print(f"{GREEN}Nginx has been started{NC}")
    else:
        print(f"{RED}Nginx binary not found{NC}")


def stop_nginx():
    """Stop Nginx"""
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY, '-s', 'stop'], check=True)
        print(f"{GREEN}Nginx has been stopped{NC}")
    else:
        print(f"{RED}Nginx binary not found{NC}")


def reload_nginx():
    """Reload Nginx configuration"""
    if os.path.isfile(NGINX_BINARY):
        subprocess.run([NGINX_BINARY, '-s', 'reload'], check=True)
        print(f"{GREEN}Nginx configuration has been reloaded{NC}")
    else:
        print(f"{RED}Nginx binary not found{NC}")


def restart_nginx():
    """Restart Nginx"""
    stop_nginx()
    start_nginx()


def check_config():
    """Check Nginx configuration syntax"""
    if os.path.isfile(NGINX_CONF):
        result = subprocess.run(
            [NGINX_BINARY, '-t', '-c', NGINX_CONF], check=True)
        if result.returncode == 0:
            print(f"{GREEN}Nginx configuration syntax is correct{NC}")
        else:
            print(f"{RED}Nginx configuration syntax is incorrect{NC}")
    else:
        print(f"{RED}Nginx configuration file not found{NC}")


def show_status():
    """Show Nginx status"""
    if subprocess.run("pgrep nginx", shell=True, stdout=subprocess.PIPE, check=True).stdout:
        print(f"{GREEN}Nginx is running{NC}")
    else:
        print(f"{RED}Nginx is not running{NC}")


def show_version():
    """Show Nginx version"""
    result = subprocess.run([NGINX_BINARY, '-v'],
                            stderr=subprocess.PIPE, check=True)
    print(result.stderr.decode())


def backup_config():
    """Backup Nginx configuration file"""
    if not os.path.exists(BACKUP_PATH):
        os.makedirs(BACKUP_PATH)
    backup_file = os.path.join(BACKUP_PATH, "nginx.conf.bak")
    shutil.copy(NGINX_CONF, backup_file)
    print(f"{GREEN}Nginx configuration file has been backed up to {backup_file}{NC}")


def restore_config():
    """Restore Nginx configuration file"""
    backup_file = os.path.join(BACKUP_PATH, "nginx.conf.bak")
    if os.path.isfile(backup_file):
        shutil.copy(backup_file, NGINX_CONF)
        print(f"{GREEN}Nginx configuration file has been restored from backup{NC}")
    else:
        print(f"{RED}Backup file not found{NC}")


def show_help():
    """Show help message"""
    print(
        "Usage: python nginx_manager.py [start|stop|reload|restart|check|status|version|backup|restore|help]")
    print("  start    Start Nginx")
    print("  stop     Stop Nginx")
    print("  reload   Reload Nginx configuration")
    print("  restart  Restart Nginx")
    print("  check    Check Nginx configuration syntax")
    print("  status   Show Nginx status")
    print("  version  Show Nginx version")
    print("  backup   Backup Nginx configuration file")
    print("  restore  Restore Nginx configuration file")
    print("  help     Show help message")


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
        print(f"{RED}Invalid command{NC}")
        show_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
