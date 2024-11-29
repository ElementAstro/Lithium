import asyncio
import subprocess
import shutil
from pathlib import Path
from typing import Union, List, Optional, Tuple, Literal
from dataclasses import dataclass, field

from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import Progress, BarColumn, TextColumn, TimeRemainingColumn
import argparse
import sys

# Define custom exception classes


class UnzipError(Exception):
    """Base exception class for UnzipWrapper."""


class UnzipExtractionError(UnzipError):
    """Exception for extraction failures."""


class UnzipListError(UnzipError):
    """Exception for listing contents failures."""


class UnzipValidationError(UnzipError):
    """Exception for parameter validation failures."""


class UnzipIntegrityError(UnzipError):
    """Exception for archive integrity test failures."""


class UnzipDeleteError(UnzipError):
    """Exception for deletion failures."""


@dataclass
class UnzipConfig:
    """Configuration for UnzipWrapper."""
    executable: str = "unzip"
    log_file: Path = Path("unzip_wrapper.log")
    log_level: str = "DEBUG"


class UnzipWrapper:
    """Wrapper class for the unzip command-line tool."""

    def __init__(self, config: UnzipConfig = UnzipConfig()):
        """
        Initialize the unzip command-line tool wrapper.

        Args:
            config (UnzipConfig): Configuration for the UnzipWrapper.
        """
        self.config = config
        self.console = Console()
        self.setup_logging()
        self._validate_executable()

    def setup_logging(self):
        """Configure Loguru for logging."""
        logger.remove()
        logger.add(
            self.config.log_file,
            rotation="5 MB",
            retention="7 days",
            enqueue=True,
            level=self.config.log_level,
            format="{time} | {level} | {message}",
        )
        logger.add(
            sys.stderr,
            level="INFO",
            format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        )
        logger.debug("Logging is configured.")

    def _validate_executable(self):
        """Validate that the unzip executable exists."""
        if not shutil.which(self.config.executable):
            logger.error(f"Executable not found: {self.config.executable}")
            raise UnzipValidationError(
                f"Executable does not exist: {self.config.executable}"
            )
        logger.info(f"Using unzip executable: {self.config.executable}")

    async def _run_command(self, args: List[str]) -> subprocess.CompletedProcess:
        """
        Asynchronously run the unzip command and capture output.

        Args:
            args (List[str]): Arguments to pass to unzip.

        Returns:
            subprocess.CompletedProcess: Completed process with return code and output.

        Raises:
            UnzipError: If the unzip command fails.
        """
        command = [self.config.executable] + args
        logger.debug(f"Running command: {' '.join(command)}")
        try:
            process = await asyncio.create_subprocess_exec(
                *command,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )
            stdout, stderr = await process.communicate()
            if process.returncode != 0:
                logger.error(
                    f"Unzip command failed with exit code {process.returncode}"
                )
                logger.error(f"Error output: {stderr.decode().strip()}")
                raise UnzipError(
                    f"Command execution failed: {' '.join(command)}"
                )
            logger.debug(f"Command output: {stdout.decode().strip()}")
            return subprocess.CompletedProcess(
                args=command,
                returncode=process.returncode,
                stdout=stdout.decode().strip(),
                stderr=stderr.decode().strip()
            )
        except Exception as e:
            logger.error(f"Failed to run unzip command: {e}")
            raise UnzipError(
                f"Failed to run command: {' '.join(command)}") from e

    def _validate_archive_exists(self, archive: Union[str, Path]) -> None:
        """
        Validate that the archive exists.

        Args:
            archive (Union[str, Path]): Path to the archive.

        Raises:
            UnzipValidationError: If the archive does not exist.
        """
        if not Path(archive).exists():
            logger.error(f"Archive does not exist: {archive}")
            raise UnzipValidationError(f"Archive does not exist: {archive}")
        logger.debug("Archive exists")

    async def extract(
        self,
        archive: Union[str, Path],
        destination: Union[str, Path],
        password: Optional[str] = None,
        force: bool = False
    ) -> None:
        """
        Extract the specified archive to the destination folder.

        Args:
            archive (Union[str, Path]): Path to the archive.
            destination (Union[str, Path]): Path to the extraction destination.
            password (Optional[str]): Password for the archive (optional).
            force (bool): Whether to forcibly overwrite the destination directory if it exists.

        Raises:
            UnzipExtractionError: If extraction fails.
        """
        logger.info(
            f"Starting extraction of {archive} to {destination} with force={force}"
        )
        self._validate_archive_exists(archive)
        destination_path = Path(destination)

        if destination_path.exists():
            if force:
                logger.warning(
                    f"Destination directory exists and will be removed: {destination}"
                )
                try:
                    shutil.rmtree(destination_path)
                    logger.info(
                        f"Removed destination directory: {destination}")
                except Exception as e:
                    logger.error(
                        f"Failed to remove destination directory: {e}")
                    raise UnzipExtractionError(
                        f"Failed to remove destination directory: {destination}"
                    ) from e
            else:
                logger.error(f"Destination directory exists: {destination}")
                raise UnzipValidationError(
                    f"Destination directory exists: {destination}"
                )

        try:
            destination_path.mkdir(parents=True, exist_ok=True)
            args = ["-o", str(archive), "-d", str(destination)]
            if password:
                args.extend(["-P", password])
            logger.debug(f"Unzip arguments: {args}")
            await self._run_command(args)
            logger.success(
                f"Successfully extracted {archive} to {destination}"
            )
            self.console.print(
                f"[bold green]Successfully extracted {archive} to {destination}[/bold green]"
            )
        except UnzipError as e:
            logger.error(f"Extraction process failed: {e}")
            raise UnzipExtractionError("Extraction failed") from e

    async def list_contents(
        self,
        archive: Union[str, Path],
        password: Optional[str] = None
    ) -> str:
        """
        List the contents of the archive.

        Args:
            archive (Union[str, Path]): Path to the archive.
            password (Optional[str]): Password for the archive (optional).

        Returns:
            str: String representation of the archive contents.

        Raises:
            UnzipListError: If listing contents fails.
        """
        logger.info(f"Listing contents of {archive}")
        self._validate_archive_exists(archive)
        args = ["-l", str(archive)]
        if password:
            args.extend(["-P", password])
        try:
            result = await self._run_command(args)
            logger.success(f"Successfully listed contents of {archive}")
            return result.stdout
        except UnzipError as e:
            logger.error(f"Listing contents failed: {e}")
            raise UnzipListError("Listing contents failed") from e

    async def test_integrity(
        self,
        archive: Union[str, Path],
        password: Optional[str] = None
    ) -> bool:
        """
        Test the integrity of the archive.

        Args:
            archive (Union[str, Path]): Path to the archive.
            password (Optional[str]): Password for the archive (optional).

        Returns:
            bool: Whether the test was successful.

        Raises:
            UnzipIntegrityError: If integrity test fails.
        """
        logger.info(f"Testing integrity of {archive}")
        self._validate_archive_exists(archive)
        args = ["-t", str(archive)]
        if password:
            args.extend(["-P", password])
        try:
            result = await self._run_command(args)
            is_valid = (
                "No errors detected" in result.stderr or "Everything is Ok" in result.stdout
            )
            if is_valid:
                logger.success(f"Integrity test passed for {archive}")
            else:
                logger.warning(f"Integrity test failed for {archive}")
            return is_valid
        except UnzipError as e:
            logger.error(f"Integrity test failed: {e}")
            raise UnzipIntegrityError("Integrity test failed") from e

    async def delete_archive(self, archive: Union[str, Path]) -> None:
        """
        Delete the specified archive.

        Args:
            archive (Union[str, Path]): Path to the archive.

        Raises:
            UnzipDeleteError: If deletion fails.
        """
        logger.info(f"Deleting archive: {archive}")
        self._validate_archive_exists(archive)
        try:
            Path(archive).unlink()
            logger.success(f"Successfully deleted archive: {archive}")
            self.console.print(
                f"[bold green]Successfully deleted archive: {archive}[/bold green]"
            )
        except Exception as e:
            logger.error(f"Failed to delete archive: {e}")
            raise UnzipDeleteError(
                f"Failed to delete archive: {archive}"
            ) from e

    async def update_archive(
        self,
        archive: Union[str, Path],
        files: List[Union[str, Path]],
        action: Literal['add', 'delete'] = 'add'
    ) -> None:
        """
        Update the archive by adding or deleting files.

        Note: The unzip tool does not support updating archives. This method is a placeholder
        and raises NotImplementedError.

        Args:
            archive (Union[str, Path]): Path to the archive.
            files (List[Union[str, Path]]): List of files to add or delete.
            action (Literal['add', 'delete']): Action to perform.

        Raises:
            NotImplementedError: Always, since unzip does not support updating archives.
        """
        logger.warning("Update operation is not supported by unzip.")
        raise NotImplementedError(
            "Update operation is not supported by unzip."
        )

    async def execute(
        self,
        action: Literal['extract', 'list', 'test', 'delete', 'update'],
        archive: Optional[Union[str, Path]] = None,
        destination: Optional[Union[str, Path]] = None,
        password: Optional[str] = None,
        force: bool = False,
        files: Optional[List[Union[str, Path]]] = None,
        update_action: Literal['add', 'delete'] = 'add'
    ) -> Optional[str]:
        """
        Execute the corresponding unzip command based on the action type.

        Args:
            action (Literal['extract', 'list', 'test', 'delete', 'update']): Action type.
            archive (Optional[Union[str, Path]]): Path to the archive.
            destination (Optional[Union[str, Path]]): Path to the extraction destination (only for 'extract').
            password (Optional[str]): Password for the archive (optional).
            force (bool): Whether to forcibly overwrite the destination directory (only for 'extract').
            files (Optional[List[Union[str, Path]]]): List of files to add or delete (only for 'update').
            update_action (Literal['add', 'delete']): Action to perform in update (only for 'update').

        Returns:
            Optional[str]: If the action is 'list', returns the archive contents; if 'test', returns integrity status; otherwise, None.

        Raises:
            UnzipError: If the action fails.
        """
        logger.info(f"Executing action: {action}")
        try:
            if action == 'extract':
                if archive is None or destination is None:
                    raise UnzipValidationError(
                        "Extract action requires 'archive' and 'destination' parameters"
                    )
                await self.extract(archive, destination, password, force)
            elif action == 'list':
                if archive is None:
                    raise UnzipValidationError(
                        "List action requires 'archive' parameter"
                    )
                return await self.list_contents(archive, password)
            elif action == 'test':
                if archive is None:
                    raise UnzipValidationError(
                        "Test action requires 'archive' parameter"
                    )
                return await self.test_integrity(archive, password)
            elif action == 'delete':
                if archive is None:
                    raise UnzipValidationError(
                        "Delete action requires 'archive' parameter"
                    )
                await self.delete_archive(archive)
            elif action == 'update':
                if archive is None or files is None:
                    raise UnzipValidationError(
                        "Update action requires 'archive' and 'files' parameters"
                    )
                await self.update_archive(archive, files, action=update_action)
            else:
                logger.error(f"Unsupported action type: {action}")
                raise UnzipValidationError(
                    f"Unsupported action type: {action}"
                )
        except UnzipError as e:
            logger.error(f"Action {action} failed: {e}")
            raise e
        return None


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Unzip Command-Line Tool Wrapper"
    )
    subparsers = parser.add_subparsers(dest='action', help='Action types')

    # Extract action
    extract_parser = subparsers.add_parser('extract', help='Extract files')
    extract_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive'
    )
    extract_parser.add_argument(
        '-d', '--destination', required=True, help='Path to the extraction destination'
    )
    extract_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)'
    )
    extract_parser.add_argument(
        '--force', action='store_true', help='Force overwrite if the destination directory exists'
    )

    # List contents action
    list_parser = subparsers.add_parser('list', help='List archive contents')
    list_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive'
    )
    list_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)'
    )

    # Test integrity action
    test_parser = subparsers.add_parser('test', help='Test archive integrity')
    test_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive'
    )
    test_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)'
    )

    # Delete action
    delete_parser = subparsers.add_parser('delete', help='Delete archive')
    delete_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive'
    )

    # Update action
    update_parser = subparsers.add_parser('update', help='Update archive')
    update_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive'
    )
    update_parser.add_argument(
        '-f', '--files', nargs='+', required=True, help='Files to add or delete'
    )
    update_parser.add_argument(
        '--action', choices=['add', 'delete'], default='add', help='Action to perform on files'
    )

    return parser.parse_args()


async def main():
    """
    Main function to execute the UnzipWrapper operations based on command-line arguments.
    """
    args = parse_args()
    console = Console()

    config = UnzipConfig()
    try:
        zipper = UnzipWrapper(config)
    except UnzipValidationError as e:
        logger.critical(e)
        console.print(f"[bold red]{e}[/bold red]")
        sys.exit(1)

    try:
        if args.action == 'extract':
            await zipper.execute(
                action='extract',
                archive=args.archive,
                destination=args.destination,
                password=args.password,
                force=args.force
            )
        elif args.action == 'list':
            content = await zipper.execute(
                action='list',
                archive=args.archive,
                password=args.password
            )
            table = Table(title=f"Contents of {args.archive}")
            table.add_column("File Name", style="cyan")
            table.add_column("Details", style="magenta")
            for line in content.splitlines()[3:-2]:  # Skip header and footer
                table.add_row(line)
            console.print(table)
        elif args.action == 'test':
            is_valid = await zipper.execute(
                action='test',
                archive=args.archive,
                password=args.password
            )
            status = "[bold green]Valid[/bold green]" if is_valid else "[bold red]Invalid[/bold red]"
            console.print(f"Integrity Test: {status}")
        elif args.action == 'delete':
            await zipper.execute(
                action='delete',
                archive=args.archive
            )
        elif args.action == 'update':
            await zipper.execute(
                action='update',
                archive=args.archive,
                files=args.files,
                update_action=args.action
            )
        else:
            logger.error(f"No valid action provided.")
            console.print("[bold red]No valid action provided.[/bold red]")
            sys.exit(1)
    except UnzipError as e:
        logger.critical(f"Operation failed: {e}")
        console.print(f"[bold red]Operation failed: {e}[/bold red]")
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
