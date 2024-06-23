import asyncio
from typing import Optional, List
from dataclasses import dataclass, field
from loguru import logger
from asyncio.subprocess import Process

@dataclass
class SubprocessManager:
    process: Optional[Process] = field(default=None, init=False)

    async def start_process(self, command: str, *args: str) -> None:
        self.process = await asyncio.create_subprocess_exec(
            command, *args,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        logger.info(f"Started process: {command} {' '.join(args)} with PID {self.process.pid}")

    async def read_stdout(self) -> Optional[str]:
        if self.process and self.process.stdout:
            line = await self.process.stdout.readline()
            if line:
                return line.decode('utf-8').strip()
        return None

    async def read_stderr(self) -> Optional[str]:
        if self.process and self.process.stderr:
            line = await self.process.stderr.readline()
            if line:
                return line.decode('utf-8').strip()
        return None

    async def stop_process(self) -> None:
        if self.process:
            self.process.terminate()
            await self.process.wait()
            logger.info(f"Stopped process: {self.process.pid}")
            self.process = None

    async def run_command(self, command: str, *args: str) -> None:
        await self.start_process(command, *args)
        stdout_task = asyncio.create_task(self.read_stdout())
        stderr_task = asyncio.create_task(self.read_stderr())

        while self.process and self.process.returncode is None:
            stdout = await stdout_task
            stderr = await stderr_task

            if stdout:
                logger.info(f"STDOUT: {stdout}")
            if stderr:
                logger.error(f"STDERR: {stderr}")

            stdout_task = asyncio.create_task(self.read_stdout())
            stderr_task = asyncio.create_task(self.read_stderr())

        await self.stop_process()

    def get_pid(self) -> Optional[int]:
        if self.process:
            return self.process.pid
        return None

    def is_running(self) -> bool:
        return self.process is not None and self.process.returncode is None

# Global instance of SubprocessManager for easy access
subprocess_manager = SubprocessManager()

# Example usage
async def main():
    await subprocess_manager.run_command("ping", "google.com", "-c", "4")

if __name__ == "__main__":
    asyncio.run(main())