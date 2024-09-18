import subprocess
import os
import sys
from pathlib import Path
import argparse
from loguru import logger


def set_ulimit(unlimited: bool):
    """
    Set the core dump size using ulimit.

    Parameters:
    unlimited (bool): If True, set the core dump size to unlimited. Otherwise, set it to 0.
    """
    size = "unlimited" if unlimited else "0"
    try:
        subprocess.run(["ulimit", "-c", size], shell=True,
                       check=True, executable='/bin/bash')
        logger.info(f"Core dump size set to {size}.")
    except subprocess.CalledProcessError as e:
        logger.error(f"Error setting ulimit: {e}")


def set_core_pattern(core_pattern: str):
    """
    Set the core pattern for core dump files.

    Parameters:
    core_pattern (str): The pattern to use for core dump files.

    Note:
    This function requires root privileges to modify /proc/sys/kernel/core_pattern.
    """
    if os.geteuid() != 0:
        logger.error(
            "Setting core pattern requires root privileges. Please run as root.")
        sys.exit(1)

    try:
        with open("/proc/sys/kernel/core_pattern", "w", encoding="utf-8") as f:
            f.write(core_pattern)
        logger.info(f"Core pattern set to: {core_pattern}")
    except PermissionError as e:
        logger.error(f"Permission denied: {
                     e}. Please run as root to set core pattern.")
        sys.exit(1)


def compile_cpp_program(source_file: str, output_file: str, compile_flags: list, cpp_standard: str):
    """
    Compile the C++ program with optional flags and standard.

    Parameters:
    source_file (str): The path to the C++ source file.
    output_file (str): The name of the output executable file.
    compile_flags (list): Additional flags for the g++ compiler.
    cpp_standard (str): The C++ standard to use (e.g., c++11, c++14, c++17, c++20).
    """
    flags = compile_flags + [f"-std={cpp_standard}"]
    try:
        subprocess.run(["g++", *flags, source_file, "-o",
                       output_file, "-g"], check=True)
        logger.info(f"Compiled {source_file} to {
                    output_file} with flags: {' '.join(flags)}.")
    except subprocess.CalledProcessError as e:
        logger.error(f"Compilation failed: {e}")
        sys.exit(1)


def run_cpp_program(executable: str, auto_analyze: bool, core_dir: str):
    """
    Run the compiled C++ program and handle crashes.

    Parameters:
    executable (str): The name of the executable file to run.
    auto_analyze (bool): If True, automatically analyze the core dump if the program crashes.
    core_dir (str): The directory to search for core dump files.
    """
    try:
        subprocess.run([f"./{executable}"], check=True)
        logger.info(f"Program {executable} ran successfully without crashing.")
    except subprocess.CalledProcessError as e:
        logger.warning(f"Program crashed: {e}")
        core_file = find_latest_core_file(core_dir)
        if auto_analyze and core_file:
            analyze_core_dump(executable, core_file)
        elif not core_file:
            logger.warning("No core dump file found.")


def find_latest_core_file(core_dir: str) -> Path:
    """
    Find the latest core dump file in the specified directory.

    Parameters:
    core_dir (str): The directory to search for core dump files.

    Returns:
    Path: The path to the latest core dump file, or None if no core dump files are found.
    """
    core_files = list(Path(core_dir).glob("core.*"))
    if not core_files:
        logger.warning("No core dump files found in the specified directory.")
        return None
    latest_core = max(core_files, key=os.path.getctime)
    logger.info(f"Found core dump file: {latest_core}")
    return latest_core


def analyze_core_dump(executable: str, core_file: Path, gdb_commands: list = ["-ex", "bt", "-ex", "quit"]):
    """
    Analyze the core dump file using gdb with custom commands.

    Parameters:
    executable (str): The name of the executable file.
    core_file (Path): The path to the core dump file.
    gdb_commands (list): A list of gdb commands to run for analysis.
    """
    try:
        result = subprocess.run(
            ["gdb", executable, str(core_file), *gdb_commands],
            text=True,
            capture_output=True,
            check=True
        )
        logger.info("Core dump analysis:")
        logger.info(result.stdout)
    except subprocess.CalledProcessError as e:
        logger.error(f"GDB analysis failed: {e}")


def configure_logging(log_file: str):
    """
    Configure the loguru logger.

    Parameters:
    log_file (str): The path to the log file. If empty, logs will only be written to stderr.
    """
    logger.remove()  # Remove the default logger
    logger.add(sys.stderr, level="INFO")
    if log_file:
        # Rotate logs every 10 MB
        logger.add(log_file, level="DEBUG", rotation="10 MB")
    logger.info("Logging is configured.")


def main():
    """
    Main function to parse arguments and run the core dump and analysis tool.
    """
    parser = argparse.ArgumentParser(
        description="C++ Core Dump and Analysis Tool with Logging")
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

    args = parser.parse_args()

    configure_logging(args.log_file)

    if args.ulimit:
        set_ulimit(True)
    set_core_pattern(args.core_pattern)

    compile_cpp_program(args.source, args.output, args.flags, args.std)
    run_cpp_program(args.output, args.auto_analyze, args.core_dir)


if __name__ == "__main__":
    main()
