import subprocess
import os
import sys
from pathlib import Path
import argparse
from loguru import logger
from typing import Optional, List, Dict


class CoreRunner:
    """
    CoreRunner handles the setup and execution of C++ programs with core dump analysis.
    """

    def __init__(self, args: argparse.Namespace):
        self.source_file: Path = Path(args.source).resolve()
        self.output_file: Path = Path(args.output).resolve()
        self.core_dir: Path = Path(args.core_dir).resolve()
        self.core_pattern: str = args.core_pattern
        self.ulimit_unlimited: bool = args.ulimit
        self.compile_flags: List[str] = args.flags or []
        self.cpp_standard: str = args.std
        self.gdb_commands: List[str] = args.gdb_commands or [
            "-ex", "bt", "-ex", "quit"]
        self.auto_analyze: bool = args.auto_analyze
        self.log_file: Optional[Path] = Path(
            args.log_file).resolve() if args.log_file else None

    def setup_logging(self) -> None:
        """
        Configure Loguru for logging.
        """
        logger.remove()
        logger.add(
            sys.stderr,
            level="INFO",
            format="<level>{message}</level>",
        )
        if self.log_file:
            logger.add(
                self.log_file,
                rotation="10 MB",
                retention="7 days",
                compression="zip",
                enqueue=True,
                encoding="utf-8",
                format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
                level="DEBUG"
            )
        logger.debug("Logging is configured.")

    def set_ulimit(self) -> None:
        """
        Set the core dump size using ulimit.
        """
        size = "unlimited" if self.ulimit_unlimited else "0"
        try:
            subprocess.run(["bash", "-c", f"ulimit -c {size}"], check=True)
            logger.info(f"Core dump size set to {size}.")
        except subprocess.CalledProcessError as e:
            logger.error(f"Error setting ulimit: {e}")
            sys.exit(1)

    def set_core_pattern(self) -> None:
        """
        Set the core pattern for core dump files.
        """
        if os.geteuid() != 0:
            logger.error(
                "Setting core pattern requires root privileges. Please run as root.")
            sys.exit(1)

        try:
            core_path = Path("/proc/sys/kernel/core_pattern")
            core_path.write_text(self.core_pattern, encoding="utf-8")
            logger.info(f"Core pattern set to: {self.core_pattern}")
        except PermissionError as e:
            logger.error(
                f"Permission denied: {e}. Please run as root to set core pattern.")
            sys.exit(1)
        except Exception as e:
            logger.error(f"Failed to set core pattern: {e}")
            sys.exit(1)

    def compile_cpp_program(self) -> None:
        """
        Compile the C++ program with optional flags and standard.
        """
        if not self.source_file.exists():
            logger.error(f"Source file not found: {self.source_file}")
            sys.exit(1)

        flags = self.compile_flags + [f"-std={self.cpp_standard}"]
        compile_cmd = ["g++", *flags,
                       str(self.source_file), "-o", str(self.output_file), "-g"]
        logger.info(f"Running compile command: {' '.join(compile_cmd)}")
        try:
            subprocess.run(compile_cmd, check=True)
            logger.success(f"Compilation successful: {self.output_file}")
        except subprocess.CalledProcessError as e:
            logger.error(f"Compilation failed: {e}")
            sys.exit(1)
        except Exception as e:
            logger.exception(f"Unexpected error during compilation: {e}")
            sys.exit(1)

    def run_cpp_program(self) -> None:
        """
        Run the compiled C++ program and handle crashes.
        """
        if not self.output_file.exists():
            logger.error(f"Executable not found: {self.output_file}")
            sys.exit(1)

        try:
            logger.info(f"Running program: {self.output_file}")
            subprocess.run([str(self.output_file)], check=True)
            logger.info(
                f"Program {self.output_file} ran successfully without crashing.")
        except subprocess.CalledProcessError as e:
            logger.warning(f"Program crashed with exit code {e.returncode}.")
            core_file = self.find_latest_core_file()
            if self.auto_analyze and core_file:
                self.analyze_core_dump(core_file)
            elif not core_file:
                logger.warning("No core dump file found.")
        except Exception as e:
            logger.exception(f"Unexpected error during program execution: {e}")
            sys.exit(1)

    def find_latest_core_file(self) -> Optional[Path]:
        """
        Find the latest core dump file in the specified directory.

        Returns:
            Optional[Path]: The path to the latest core dump file, or None if no core dump files are found.
        """
        if not self.core_dir.exists():
            logger.warning(f"Core directory does not exist: {self.core_dir}")
            return None

        core_files = list(self.core_dir.glob("core.*"))
        if not core_files:
            logger.warning(
                "No core dump files found in the specified directory.")
            return None

        latest_core = max(core_files, key=lambda f: f.stat().st_ctime)
        logger.info(f"Found core dump file: {latest_core}")
        return latest_core

    def analyze_core_dump(self, core_file: Path) -> None:
        """
        Analyze the core dump file using gdb with custom commands.

        Parameters:
            core_file (Path): The path to the core dump file.
        """
        gdb_cmd = ["gdb", str(self.output_file), str(
            core_file), *self.gdb_commands]
        logger.info(f"Running GDB with command: {' '.join(gdb_cmd)}")
        try:
            result = subprocess.run(
                gdb_cmd,
                text=True,
                capture_output=True,
                check=True
            )
            logger.info("Core dump analysis output:")
            logger.info(result.stdout)
        except subprocess.CalledProcessError as e:
            logger.error(f"GDB analysis failed: {e.stderr}")
        except Exception as e:
            logger.exception(f"Unexpected error during GDB analysis: {e}")

    def run(self) -> None:
        """
        Execute the full workflow: setup, compile, and run the program.
        """
        logger.debug("Starting CoreRunner workflow.")
        if self.ulimit_unlimited:
            self.set_ulimit()
        self.set_core_pattern()
        self.compile_cpp_program()
        self.run_cpp_program()
        logger.debug("CoreRunner workflow completed successfully.")


def configure_logging(log_file: str) -> None:
    """
    Configure the loguru logger.

    Parameters:
        log_file (str): The path to the log file. If empty, logs will only be written to stderr.
    """
    logger.remove()  # Remove the default logger
    logger.add(
        sys.stderr,
        level="INFO",
        format="<level>{message}</level>",
    )
    if log_file:
        logger.add(
            log_file,
            rotation="10 MB",
            retention="7 days",
            compression="zip",
            enqueue=True,
            encoding="utf-8",
            format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
            level="DEBUG"
        )
    logger.debug("Logging is configured.")


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="C++ Core Dump and Analysis Tool with Enhanced Logging and Exception Handling"
    )
    parser.add_argument("source", help="C++ source file to compile and run")
    parser.add_argument("-o", "--output", default="a.out",
                        help="Output executable name")
    parser.add_argument("-d", "--core-dir", default="/tmp",
                        help="Directory to search for core dumps")
    parser.add_argument("-p", "--core-pattern",
                        default="/tmp/core.%e.%p", help="Core pattern for dump files")
    parser.add_argument("-u", "--ulimit", action="store_true",
                        help="Set core dump size to unlimited")
    parser.add_argument("-f", "--flags", nargs='*', default=[],
                        help="Additional flags for g++ compilation")
    parser.add_argument("-s", "--std", default="c++17",
                        help="C++ standard to use (e.g., c++11, c++14, c++17, c++20)")
    parser.add_argument("-g", "--gdb-commands", nargs='*', default=[
                        "-ex", "bt", "-ex", "quit"], help="GDB commands for core dump analysis")
    parser.add_argument("-a", "--auto-analyze", action="store_true",
                        help="Automatically analyze core dump if program crashes")
    parser.add_argument("-l", "--log-file", default="",
                        help="Log file to write logs to")
    return parser.parse_args()


def main():
    """
    Main function to run the CoreRunner.
    """
    args = parse_arguments()
    configure_logging(args.log_file)
    runner = CoreRunner(args)
    runner.run()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        sys.exit(0)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        sys.exit(1)
