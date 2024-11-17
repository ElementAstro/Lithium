import argparse
import os
import signal
import subprocess
import sys
import time
from datetime import datetime

import psutil
from loguru import logger

# Configure Loguru
logger.remove()  # Remove the default logger
logger.add(
    "/tmp/daemon.log",
    rotation="10 MB",        # Rotate log file when it reaches 10MB
    retention="10 days",     # Retain logs for the last 10 days
    level="INFO",
    format="{time:YYYY-MM-DD at HH:mm:ss} | {level} | {message}"
)

# Daemon configuration parameters
DEFAULT_CONFIG = {
    "process_name": "python",        # Name of the process to monitor
    "script_path": "target_script.py",  # Path to the target script
    "restart_interval": 5,           # Restart interval in seconds
    "cpu_threshold": 80,             # CPU usage threshold in percentage
    "memory_threshold": 500,         # Memory usage threshold in MB
    "max_restarts": 3,               # Maximum number of restarts
    "monitor_interval": 5            # Monitoring interval in seconds
}

PID_FILE = "/tmp/daemon.pid"


class DaemonProcess:
    def __init__(self, config):
        self.config = config
        self.restart_count = 0
        self.process = None

        # Check if the target script exists
        if not os.path.isfile(self.config["script_path"]):
            logger.critical(
                f"Target script does not exist: {self.config['script_path']}")
            raise FileNotFoundError(
                f"Target script does not exist: {self.config['script_path']}")

    def start_target_process(self):
        """
        Start the target process.
        """
        try:
            self.process = subprocess.Popen(
                [sys.executable, self.config["script_path"]],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            logger.info(f"Target process started, PID: {self.process.pid}")
        except Exception as e:
            logger.exception(f"Failed to start target process: {e}")
            raise

    def is_process_running(self):
        """
        Check if the target process is running.
        :return: Boolean indicating if the process exists.
        """
        if self.process is None:
            logger.debug("Target process instance is None.")
            return False

        # Check if the process is still running
        running = self.process.poll() is None
        logger.debug(
            f"Process status: {'Running' if running else 'Stopped'} (PID: {self.process.pid})")
        return running

    def monitor_process_health(self):
        """
        Monitor the CPU and memory usage of the process. Restart if thresholds are exceeded.
        """
        try:
            proc = psutil.Process(self.process.pid)
            cpu_usage = proc.cpu_percent(interval=1)
            memory_usage = proc.memory_info().rss / (1024 * 1024)  # Convert to MB
            logger.info(
                f"Process PID: {proc.pid}, CPU: {cpu_usage}%, Memory: {memory_usage:.2f}MB")

            # Check if CPU and memory usage exceed thresholds
            if cpu_usage > self.config["cpu_threshold"]:
                logger.warning(
                    f"CPU usage exceeded threshold ({cpu_usage}% > {self.config['cpu_threshold']}%), restarting process...")
                self.restart_process()

            if memory_usage > self.config["memory_threshold"]:
                logger.warning(
                    f"Memory usage exceeded threshold ({memory_usage}MB > {self.config['memory_threshold']}MB), restarting process...")
                self.restart_process()

        except psutil.NoSuchProcess:
            logger.warning(
                "Process does not exist, may have crashed. Will restart...")
            self.restart_process()
        except psutil.AccessDenied:
            logger.error("Access denied when accessing process information.")
        except Exception as e:
            logger.exception(
                f"Unknown error occurred while monitoring process health: {e}")

    def restart_process(self):
        """
        Restart the target process, track restart counts, and check against maximum restarts.
        """
        if self.restart_count < self.config["max_restarts"]:
            try:
                # Terminate existing process
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

                # Increment restart count and restart process
                self.restart_count += 1
                logger.info(
                    f"Restarting target process (Restart count: {self.restart_count})")
                time.sleep(self.config["restart_interval"])
                self.start_target_process()
            except Exception as e:
                logger.exception(f"Failed to restart process: {e}")
        else:
            logger.error("Maximum restart count reached, stopping daemon.")
            self.cleanup()
            sys.exit("Daemon terminated: exceeded maximum restart count.")

    def monitor_loop(self):
        """
        Main loop of the daemon for continuous monitoring of the target process.
        """
        self.start_target_process()

        try:
            while True:
                # Check if the process is running
                if not self.is_process_running():
                    logger.warning(
                        "Target process is not running, restarting...")
                    self.restart_process()
                else:
                    # Monitor the health of the process
                    self.monitor_process_health()

                # Check at specified monitoring intervals
                time.sleep(self.config["monitor_interval"])
        except KeyboardInterrupt:
            logger.info("Daemon received interrupt signal, exiting...")
        except Exception as e:
            logger.exception(f"Error occurred while running daemon: {e}")
        finally:
            self.cleanup()

    def cleanup(self):
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

        # Remove PID file
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
            logger.debug("PID file removed.")


def write_pid():
    """
    Write the daemon's PID to the PID file.
    """
    pid = os.getpid()
    with open(PID_FILE, 'w', encoding='utf-8') as f:
        f.write(str(pid))
    logger.debug(f"PID {pid} written to {PID_FILE}")


def read_pid():
    """
    Read the PID from the PID file.
    :return: PID as an integer.
    """
    try:
        with open(PID_FILE, 'r', encoding='utf-8') as f:
            pid = int(f.read().strip())
        return pid
    except Exception as e:
        logger.error(f"Failed to read PID file: {e}")
        return None


def is_daemon_running(config):
    """
    Check if the daemon is currently running.
    :param config: Configuration dictionary.
    :return: Boolean indicating if the daemon is running.
    """
    pid = read_pid()
    if pid and psutil.pid_exists(pid):
        try:
            proc = psutil.Process(pid)
            if proc.name() == config["process_name"]:
                return True
        except psutil.NoSuchProcess:
            return False
    return False


def stop_daemon():
    """
    Stop the daemon process.
    """
    pid = read_pid()
    if not pid:
        logger.error(
            "PID file not found or PID is invalid. Daemon may not be running.")
        print("Daemon is not running.")
        return

    try:
        proc = psutil.Process(pid)
        logger.info(f"Sending SIGTERM signal to daemon PID: {pid}")
        proc.send_signal(signal.SIGTERM)
        proc.wait(timeout=10)
        logger.info("Daemon has been stopped.")
        print("Daemon has been stopped.")
    except psutil.NoSuchProcess:
        logger.error("Specified daemon process does not exist.")
        print("Daemon does not exist.")
    except psutil.TimeoutExpired:
        logger.warning("Daemon did not respond, sending SIGKILL signal.")
        proc.kill()
        print("Daemon has been forcefully stopped.")
    except Exception as e:
        logger.exception(f"Error occurred while stopping daemon: {e}")
        print(f"Error occurred while stopping daemon: {e}")
    finally:
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
            logger.debug("PID file removed.")


def start_daemon(config):
    """
    Start the daemon process.
    """
    if is_daemon_running(config):
        logger.error("Daemon is already running.")
        print("Daemon is already running.")
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
    daemon.monitor_loop()


def status_daemon():
    """
    Check the status of the daemon process.
    """
    if is_daemon_running(DEFAULT_CONFIG):
        pid = read_pid()
        logger.info(f"Daemon is running, PID: {pid}")
        print(f"Daemon is running, PID: {pid}")
    else:
        logger.info("Daemon is not running.")
        print("Daemon is not running.")


def parse_arguments():
    """
    Parse command-line arguments.
    :return: argparse.Namespace
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
        "--script_path", type=str, default=DEFAULT_CONFIG["script_path"],
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


if __name__ == "__main__":
    args = parse_arguments()

    if args.command == "start":
        config = {
            "process_name": args.process_name,
            "script_path": args.script_path,
            "restart_interval": args.restart_interval,
            "cpu_threshold": args.cpu_threshold,
            "memory_threshold": args.memory_threshold,
            "max_restarts": args.max_restarts,
            "monitor_interval": args.monitor_interval
        }
        try:
            start_daemon(config)
        except Exception as e:
            logger.critical(f"Failed to start daemon: {e}")
            sys.exit(1)
    elif args.command == "stop":
        stop_daemon()
    elif args.command == "status":
        status_daemon()
    else:
        print("Use -h for help.")
        sys.exit(1)
