import asyncio
import pytest
from app.utils.process import SubprocessManager

@pytest.fixture
def subprocess_manager():
    return SubprocessManager()

@pytest.mark.asyncio
async def test_start_process(subprocess_manager):
    await subprocess_manager.start_process("echo", "Hello, World!")
    assert subprocess_manager.is_running()
    pid = subprocess_manager.get_pid()
    assert pid is not None
    await subprocess_manager.stop_process()

@pytest.mark.asyncio
async def test_read_stdout(subprocess_manager):
    await subprocess_manager.start_process("echo", "Hello, World!")
    stdout = await subprocess_manager.read_stdout()
    assert stdout == "Hello, World!"
    await subprocess_manager.stop_process()

@pytest.mark.asyncio
async def test_read_stderr(subprocess_manager):
    await subprocess_manager.start_process("sh", "-c", "echo 'Error message' >&2")
    stderr = await subprocess_manager.read_stderr()
    assert stderr == "Error message"
    await subprocess_manager.stop_process()

@pytest.mark.asyncio
async def test_stop_process(subprocess_manager):
    await subprocess_manager.start_process("sleep", "1")
    assert subprocess_manager.is_running()
    await subprocess_manager.stop_process()
    assert not subprocess_manager.is_running()

@pytest.mark.asyncio
async def test_run_command(subprocess_manager):
    await subprocess_manager.run_command("echo", "Running command!")
    assert not subprocess_manager.is_running()

@pytest.mark.asyncio
async def test_get_pid(subprocess_manager):
    await subprocess_manager.start_process("echo", "Testing PID")
    pid = subprocess_manager.get_pid()
    assert pid is not None
    await subprocess_manager.stop_process()
    assert subprocess_manager.get_pid() is None
