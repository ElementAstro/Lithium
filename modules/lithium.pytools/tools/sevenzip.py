from __future__ import annotations
import subprocess
from pathlib import Path
from typing import Union, List, Optional, Literal
from loguru import logger
from rich.console import Console
from rich.progress import Progress, SpinnerColumn, TextColumn
from rich.prompt import Prompt
import shutil
import argparse
import sys
import dataclasses
import shlex

# Define custom exception classes


class SevenZipError(Exception):
    """Base exception class for SevenZipWrapper"""


class SevenZipCompressionError(SevenZipError):
    """Exception for compression failures"""


class SevenZipExtractionError(SevenZipError):
    """Exception for extraction failures"""


class SevenZipListError(SevenZipError):
    """Exception for listing contents failures"""


class SevenZipTestError(SevenZipError):
    """Exception for testing archive failures"""


class SevenZipValidationError(SevenZipError):
    """Exception for parameter validation failures"""


# Define supported operation types for 7z
Action = Literal['compress', 'extract', 'list',
                 'test', 'delete', 'update', 'version', 'formats']

console = Console()


@dataclasses.dataclass
class SevenZipWrapper:
    """
    A wrapper class for the 7z command-line tool, providing simplified methods
    for common archive operations like compressing, extracting, listing contents,
    and testing archives. Enhanced with logging and rich terminal outputs.
    """
    executable: str = "7z"

    def __post_init__(self):
        """
        Post-initialization to validate the 7z executable.
        """
        if not shutil.which(self.executable):
            logger.error(f"Executable not found: {self.executable}")
            raise SevenZipValidationError(
                f"Executable does not exist: {self.executable}")
        logger.info(f"Using 7z executable: {self.executable}")

    def _run_command(self, args: List[str]) -> subprocess.CompletedProcess:
        """
        Run the 7z command and capture output.

        :param args: Arguments to pass to 7z.
        :return: subprocess.CompletedProcess object containing the return code and output information.
        """
        command = [self.executable] + args
        logger.debug(
            f"Running command: {' '.join(shlex.quote(arg) for arg in command)}")
        try:
            result = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True
            )
            logger.debug(f"Command output: {result.stdout}")
            logger.debug(f"Command error output: {result.stderr}")
            return result
        except subprocess.CalledProcessError as e:
            logger.error(f"7z command failed with exit code {e.returncode}")
            logger.error(f"Error output: {e.stderr}")
            raise SevenZipError(
                f"Command execution failed: {' '.join(command)}") from e

    def _validate_files_exist(self, files: List[Union[str, Path]]) -> None:
        """
        Validate that all files exist.

        :param files: List of file paths.
        """
        missing_files = [str(file)
                         for file in files if not Path(file).exists()]
        if missing_files:
            for file in missing_files:
                logger.error(f"File does not exist: {file}")
            raise SevenZipValidationError(
                f"These files do not exist: {', '.join(missing_files)}")
        logger.debug("All files to be processed exist")

    def _validate_archive_exists(self, archive: Union[str, Path]) -> None:
        """
        Validate that the archive exists.

        :param archive: Path to the archive.
        """
        if not Path(archive).exists():
            logger.error(f"Archive does not exist: {archive}")
            raise SevenZipValidationError(f"Archive does not exist: {archive}")
        logger.debug("Archive exists")

    def get_version(self) -> str:
        """
        Get the version of the 7z executable.

        :return: Version string of 7z.
        """
        logger.info("Getting 7z version")
        result = self._run_command(["--version"])
        version_info = result.stdout.strip()
        logger.debug(f"7z version: {version_info}")
        return version_info

    def list_supported_formats(self) -> str:
        """
        List the supported archive formats by 7z.

        :return: String of supported formats.
        """
        logger.info("Listing supported archive formats")
        result = self._run_command(["i"])
        formats_info = result.stdout
        logger.debug("Supported formats retrieved")
        return formats_info

    def compress(self, files: List[Union[str, Path]], archive: Union[str, Path], level: int = 5,
                 password: Optional[str] = None, format: str = "7z", exclude: Optional[List[str]] = None) -> None:
        """
        Compress files into the specified archive.

        :param files: List of files to compress.
        :param archive: Path to the archive.
        :param level: Compression level (0-9), default is 5.
        :param password: Password for the archive (optional).
        :param format: Archive format (e.g., '7z', 'zip'), default is '7z'.
        :param exclude: List of file patterns to exclude (optional).
        """
        self._validate_files_exist(files)
        args = ["a", f"-t{format}", f"-mx={level}", str(archive)]
        if password:
            # Encrypt archive with password and filenames
            args.extend([f"-p{password}", "-mhe=on"])
        if exclude:
            for pattern in exclude:
                args.extend(["-x!" + pattern])
        args.extend(str(f) for f in files)
        logger.info(
            f"Compressing {files} into {archive} with compression level {level} and format {format}")

        with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
            task = progress.add_task("[green]Compressing...", total=None)
            try:
                self._run_command(args)
                progress.update(
                    task, description="[bold green]Compression completed")
                logger.success(
                    f"Successfully compressed {files} into {archive}")
            except SevenZipError as e:
                progress.update(
                    task, description="[bold red]Compression failed")
                logger.error(f"Compression process failed: {e}")
                raise SevenZipCompressionError("Compression failed") from e

    def extract(self, archive: Union[str, Path], destination: Union[str, Path],
                password: Optional[str] = None, force: bool = False, overwrite: bool = False) -> None:
        """
        Extract the specified archive to the destination folder.

        :param archive: Path to the archive.
        :param destination: Path to the extraction destination.
        :param password: Password for the archive (optional).
        :param force: Whether to forcibly overwrite the destination directory if it exists.
        :param overwrite: Whether to overwrite existing files.
        """
        self._validate_archive_exists(archive)
        destination_path = Path(destination)

        if destination_path.exists():
            if force:
                logger.warning(
                    f"Destination directory exists and will be removed: {destination}")
                shutil.rmtree(destination_path)
                logger.info(f"Removed destination directory: {destination}")
            else:
                logger.warning(f"Destination directory exists: {destination}")
        else:
            destination_path.mkdir(parents=True, exist_ok=True)

        args = ["x", str(archive), f"-o{destination}"]
        if overwrite:
            args.append("-y")
        if password:
            args.append(f"-p{password}")
        logger.info(f"Extracting {archive} to {destination}")

        with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
            task = progress.add_task("[green]Extracting...", total=None)
            try:
                self._run_command(args)
                progress.update(
                    task, description="[bold green]Extraction completed")
                logger.success(
                    f"Successfully extracted {archive} to {destination}")
            except SevenZipError as e:
                progress.update(
                    task, description="[bold red]Extraction failed")
                logger.error(f"Extraction process failed: {e}")
                raise SevenZipExtractionError("Extraction failed") from e

    def list_contents(self, archive: Union[str, Path],
                      password: Optional[str] = None) -> str:
        """
        List the contents of the archive.

        :param archive: Path to the archive.
        :param password: Password for the archive (optional).
        :return: String representation of the archive contents.
        """
        self._validate_archive_exists(archive)
        args = ["l", str(archive)]
        if password:
            args.append(f"-p{password}")
        logger.info(f"Listing contents of {archive}")
        try:
            result = self._run_command(args)
            logger.success(f"Successfully listed contents of {archive}")
            return result.stdout
        except SevenZipError as e:
            logger.error(f"Listing contents failed: {e}")
            raise SevenZipListError("Listing contents failed") from e

    def test_archive(self, archive: Union[str, Path],
                     password: Optional[str] = None) -> bool:
        """
        Test the integrity of the archive.

        :param archive: Path to the archive.
        :param password: Password for the archive (optional).
        :return: Whether the test was successful.
        """
        self._validate_archive_exists(archive)
        args = ["t", str(archive)]
        if password:
            args.append(f"-p{password}")
        logger.info(f"Testing integrity of {archive}")

        with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}")) as progress:
            task = progress.add_task("[green]Testing...", total=None)
            try:
                result = self._run_command(args)
                is_valid = "Everything is Ok" in result.stdout
                if is_valid:
                    progress.update(
                        task, description="[bold green]Integrity test passed")
                    logger.success(f"Integrity test passed for {archive}")
                else:
                    progress.update(
                        task, description="[bold red]Integrity test failed")
                    logger.warning(f"Integrity test failed for {archive}")
                return is_valid
            except SevenZipError as e:
                progress.update(
                    task, description="[bold red]Integrity test failed")
                logger.error(f"Integrity test failed: {e}")
                raise SevenZipTestError("Integrity test failed") from e

    def delete_archive(self, archive: Union[str, Path]) -> None:
        """
        Delete the specified archive.

        :param archive: Path to the archive.
        """
        self._validate_archive_exists(archive)
        try:
            Path(archive).unlink()
            logger.info(f"Deleted archive: {archive}")
            logger.success(f"Successfully deleted {archive}")
        except Exception as e:
            logger.error(f"Failed to delete archive: {e}")
            raise SevenZipError(f"Failed to delete archive: {archive}") from e

    def update_archive(self, archive: Union[str, Path], files: List[Union[str, Path]],
                       add: bool = True, delete: bool = False) -> None:
        """
        Update the archive by adding or deleting files.

        :param archive: Path to the archive.
        :param files: List of files to add or delete.
        :param add: Whether to add files.
        :param delete: Whether to delete files.
        """
        self._validate_archive_exists(archive)
        if add:
            self._validate_files_exist(files)
            args = ["u", str(archive)]
            args.extend(str(f) for f in files)
            logger.info(f"Adding files to {archive}: {files}")
            try:
                self._run_command(args)
                logger.success(f"Successfully added files to {archive}")
            except SevenZipError as e:
                logger.error(f"Failed to update archive: {e}")
                raise SevenZipError("Failed to update archive") from e
        if delete:
            args = ["d", str(archive)]
            args.extend(str(f) for f in files)
            logger.info(f"Deleting files from {archive}: {files}")
            try:
                self._run_command(args)
                logger.success(f"Successfully deleted files from {archive}")
            except SevenZipError as e:
                logger.error(f"Failed to update archive: {e}")
                raise SevenZipError("Failed to update archive") from e

    def execute(self, action: Action, **kwargs) -> Optional[Union[str, bool]]:
        """
        Execute the corresponding 7z command based on the action type.

        :param action: Action type ('compress', 'extract', 'list', 'test', 'delete', 'update').
        :param kwargs: Keyword arguments for the specific action.
        :return: If the action is 'list', returns the archive contents; 'test' returns a boolean.
        """
        logger.info(f"Executing action: {action}")
        try:
            match action:
                case 'compress':
                    self.compress(**kwargs)
                case 'extract':
                    self.extract(**kwargs)
                case 'list':
                    return self.list_contents(**kwargs)
                case 'test':
                    return self.test_archive(**kwargs)
                case 'delete':
                    self.delete_archive(**kwargs.get('archive'))
                case 'update':
                    self.update_archive(**kwargs)
                case 'version':
                    return self.get_version()
                case 'formats':
                    return self.list_supported_formats()
                case _:
                    raise SevenZipValidationError(
                        f"Unsupported action type: {action}")
        except SevenZipError as e:
            logger.error(f"Action {action} failed: {e}")
            raise e


def main():
    parser = argparse.ArgumentParser(
        description="7z Command-Line Tool Wrapper")
    subparsers = parser.add_subparsers(dest='action', help='Action types')

    # Compress
    compress_parser = subparsers.add_parser('compress', help='Compress files')
    compress_parser.add_argument(
        '-f', '--files', nargs='+', required=True, help='List of files to compress')
    compress_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    compress_parser.add_argument('-l', '--level', type=int, default=5, choices=range(0, 10),
                                 help='Compression level (0-9), default is 5')
    compress_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')
    compress_parser.add_argument(
        '--format', default='7z', help='Archive format (e.g., 7z, zip)')
    compress_parser.add_argument(
        '-x', '--exclude', nargs='*', help='Patterns to exclude')

    # Extract
    extract_parser = subparsers.add_parser('extract', help='Extract files')
    extract_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    extract_parser.add_argument(
        '-d', '--destination', required=True, help='Path to the extraction destination')
    extract_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')
    extract_parser.add_argument(
        '--force', action='store_true', help='Force overwrite if the destination directory exists')
    extract_parser.add_argument(
        '--overwrite', action='store_true', help='Overwrite existing files')

    # List contents
    list_parser = subparsers.add_parser('list', help='List archive contents')
    list_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    list_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')

    # Test integrity
    test_parser = subparsers.add_parser('test', help='Test archive integrity')
    test_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    test_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')

    # Delete archive
    delete_parser = subparsers.add_parser('delete', help='Delete archive')
    delete_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')

    # Update archive
    update_parser = subparsers.add_parser('update', help='Update archive')
    update_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    update_parser.add_argument(
        '-f', '--files', nargs='+', required=True, help='List of files to add or delete')
    update_group = update_parser.add_mutually_exclusive_group(required=True)
    update_group.add_argument('--add', action='store_true', help='Add files')
    update_group.add_argument(
        '--delete', action='store_true', help='Delete files')

    # Get version
    version_parser = subparsers.add_parser('version', help='Get 7z version')

    # List supported formats
    formats_parser = subparsers.add_parser(
        'formats', help='List supported archive formats')

    args = parser.parse_args()

    if not args.action:
        parser.print_help()
        sys.exit(1)

    try:
        zipper = SevenZipWrapper()
    except SevenZipValidationError as e:
        logger.critical(e)
        sys.exit(1)

    try:
        if args.action == 'compress':
            zipper.execute(
                action='compress',
                files=args.files,
                archive=args.archive,
                level=args.level,
                password=args.password,
                format=args.format,
                exclude=args.exclude
            )
        elif args.action == 'extract':
            zipper.execute(
                action='extract',
                archive=args.archive,
                destination=args.destination,
                password=args.password,
                force=args.force,
                overwrite=args.overwrite
            )
        elif args.action == 'list':
            content = zipper.execute(
                action='list',
                archive=args.archive,
                password=args.password
            )
            console.print(
                f"[bold magenta]Archive Contents:[/bold magenta]\n{content}")
        elif args.action == 'test':
            is_valid = zipper.execute(
                action='test',
                archive=args.archive,
                password=args.password
            )
            if is_valid:
                console.print("[bold green]The archive is valid[/bold green]")
            else:
                console.print(
                    "[bold red]The archive is invalid or corrupted[/bold red]")
        elif args.action == 'delete':
            zipper.execute(
                action='delete',
                archive=args.archive
            )
        elif args.action == 'update':
            zipper.execute(
                action='update',
                archive=args.archive,
                files=args.files,
                add=args.add,
                delete=args.delete
            )
        elif args.action == 'version':
            version_info = zipper.execute(action='version')
            console.print(f"[bold blue]7z Version:[/bold blue] {version_info}")
        elif args.action == 'formats':
            formats_info = zipper.execute(action='formats')
            console.print(
                f"[bold blue]Supported Formats:[/bold blue]\n{formats_info}")
    except SevenZipError as e:
        logger.error(e)
        console.print(f"[bold red]Error:[/bold red] {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
