import asyncio
import subprocess
from typing import List, Optional, Callable, Dict
from loguru import logger


def start_executable(
    path: str,
    args: Optional[List[str]] = None,
    timeout: Optional[int] = None,
    env: Optional[Dict[str, str]] = None,
    stdin: Optional[str] = None,
    priority: Optional[int] = None
):
    """
    Starts an executable as a subprocess with error handling and optional arguments.
    """
    try:
        command = [path] + (args or [])
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE if stdin else None,
            env=env,
            text=True
        )

        if priority:
            try:
                process.nice(priority)
            except AttributeError:
                logger.warning(
                    "Setting process priority is not supported on this platform")

        if stdin:
            process.stdin.write(stdin)
            process.stdin.flush()

        if timeout:
            asyncio.create_task(terminate_after_timeout(process, timeout))

        return process
    except FileNotFoundError as e:
        logger.error(f"Executable not found: {e}")
        raise


async def terminate_after_timeout(process, timeout: int):
    """
    Terminates the process after a given timeout.
    """
    await asyncio.sleep(timeout)
    if process.poll() is None:  # Process is still running
        logger.warning(f"Terminating process {process.pid} due to timeout")
        process.terminate()


async def read_process_output(process, callback: Optional[Callable[[str], None]] = None):
    """
    Reads the standard output from the subprocess and logs it or passes it to a callback.
    """
    while True:
        if process.stdout:
            output = process.stdout.readline()
            if output:
                logger.info(f"childProcess output: {output.strip()}")
                if callback:
                    callback(output.strip())
        await asyncio.sleep(0.1)


async def read_process_error(process, callback: Optional[Callable[[str], None]] = None):
    """
    Reads the error output from the subprocess and logs it or passes it to a callback.
    """
    while True:
        if process.stderr:
            error = process.stderr.readline()
            if error:
                logger.error(f"childProcess error: {error.strip()}")
                if callback:
                    callback(error.strip())
        await asyncio.sleep(0.1)


def terminate_process(process):
    """
    Terminates the subprocess.
    """
    if process.poll() is None:  # Process is still running
        logger.info(f"Terminating process {process.pid}")
        process.terminate()


def is_process_running(process) -> bool:
    """
    Checks if the subprocess is still running.
    """
    return process.poll() is None


def restart_process(process, path: str, args: Optional[List[str]] = None, env: Optional[Dict[str, str]] = None, stdin: Optional[str] = None, priority: Optional[int] = None):
    """
    Restarts the subprocess.
    """
    terminate_process(process)
    return start_executable(path, args, env=env, stdin=stdin, priority=priority)


# Example usage
if __name__ == "__main__":
    async def main():
        process = start_executable("example.exe", args=["--example-arg"], timeout=10, env={
                                   "EXAMPLE_ENV": "value"}, stdin="example input", priority=10)

        await asyncio.gather(
            read_process_output(process),
            read_process_error(process)
        )

    asyncio.run(main())
