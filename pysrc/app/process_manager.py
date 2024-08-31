# process_manager.py
import asyncio
import logging
import subprocess

logger = logging.getLogger(__name__)

def start_executable(path: str):
    """
    Starts an executable as a subprocess with error handling.
    """
    try:
        process = subprocess.Popen(
            path, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return process
    except FileNotFoundError as e:
        logger.error(f"Executable not found: {e}")
        raise

async def read_process_output(process):
    """
    Reads the standard output from the subprocess and logs it.
    """
    while True:
        if process.stdout:
            output = process.stdout.readline()
            if output:
                logger.info(f"childProcess output: {output.strip()}")
        await asyncio.sleep(0.1)

async def read_process_error(process):
    """
    Reads the error output from the subprocess and logs it.
    """
    while True:
        if process.stderr:
            error = process.stderr.readline()
            if error:
                logger.error(f"childProcess error: {error.strip()}")
        await asyncio.sleep(0.1)
