import subprocess
from pathlib import Path
from typing import Union, List, Optional, Literal
from loguru import logger
import shutil
import argparse
import sys

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
Action = Literal['compress', 'extract', 'list', 'test', 'delete', 'update']


class SevenZipWrapper:
    def __init__(self, executable: str = "7z"):
        """
        Initialize the 7z command-line tool wrapper
        :param executable: Path to the 7z executable, defaults to "7z" in the system PATH
        """
        self.executable = executable
        if not shutil.which(self.executable):
            logger.error(f"Executable not found: {self.executable}")
            raise SevenZipValidationError(f"Executable does not exist: {self.executable}")
        logger.info(f"Using 7z executable: {self.executable}")

    def _run_command(self, args: List[str]) -> subprocess.CompletedProcess:
        """
        Run the 7z command and capture output
        :param args: Arguments to pass to 7z
        :return: subprocess.CompletedProcess object containing the return code and output information
        """
        command = [self.executable] + args
        logger.debug(f"Running command: {' '.join(command)}")
        try:
            result = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True
            )
            logger.debug(f"Command output: {result.stdout}")
            return result
        except subprocess.CalledProcessError as e:
            logger.error(f"7z command failed with exit code {e.returncode}")
            logger.error(f"Error output: {e.stderr}")
            raise SevenZipError(f"Command execution failed: {' '.join(command)}") from e

    def _validate_files_exist(self, files: List[Union[str, Path]]) -> None:
        """
        Validate that all files exist
        :param files: List of file paths
        """
        for file in files:
            if not Path(file).exists():
                logger.error(f"File does not exist: {file}")
                raise SevenZipValidationError(f"File does not exist: {file}")
        logger.debug("All files to be compressed exist")

    def _validate_archive_exists(self, archive: Union[str, Path]) -> None:
        """
        Validate that the archive exists
        :param archive: Path to the archive
        """
        if not Path(archive).exists():
            logger.error(f"Archive does not exist: {archive}")
            raise SevenZipValidationError(f"Archive does not exist: {archive}")
        logger.debug("Archive exists")

    def compress(self, files: List[Union[str, Path]], archive: Union[str, Path], level: int = 5,
                 password: Optional[str] = None) -> None:
        """
        Compress files into the specified archive
        :param files: List of files to compress
        :param archive: Path to the archive
        :param level: Compression level (0-9), default is 5
        :param password: Password for the archive (optional)
        """
        self._validate_files_exist(files)
        args = ["a", "-t7z", f"-mx={level}", str(archive)]
        if password:
            args.extend(["-p" + password, "-mhe=on"])  # Encrypt archive with password and filenames
        args.extend(str(f) for f in files)
        logger.info(f"Compressing {files} into {archive} with compression level {level}")
        try:
            self._run_command(args)
            logger.success(f"Successfully compressed {files} into {archive}")
        except SevenZipError as e:
            logger.error(f"Compression process failed: {e}")
            raise SevenZipCompressionError("Compression failed") from e

    def extract(self, archive: Union[str, Path], destination: Union[str, Path],
                password: Optional[str] = None, force: bool = False) -> None:
        """
        Extract the specified archive to the destination folder
        :param archive: Path to the archive
        :param destination: Path to the extraction destination
        :param password: Password for the archive (optional)
        :param force: Whether to forcibly overwrite the destination directory if it exists
        """
        self._validate_archive_exists(archive)
        destination_path = Path(destination)

        if destination_path.exists():
            if force:
                logger.warning(f"Destination directory exists and will be removed: {destination}")
                shutil.rmtree(destination_path)
                logger.info(f"Removed destination directory: {destination}")
            else:
                logger.error(f"Destination directory exists: {destination}")
                raise SevenZipValidationError(f"Destination directory exists: {destination}")

        destination_path.parent.mkdir(parents=True, exist_ok=True)
        args = ["x", str(archive), f"-o{destination}"]
        if password:
            args.append("-p" + password)
        logger.info(f"Extracting {archive} to {destination}")
        try:
            self._run_command(args)
            logger.success(f"Successfully extracted {archive} to {destination}")
        except SevenZipError as e:
            logger.error(f"Extraction process failed: {e}")
            raise SevenZipExtractionError("Extraction failed") from e

    def list_contents(self, archive: Union[str, Path],
                      password: Optional[str] = None) -> str:
        """
        List the contents of the archive
        :param archive: Path to the archive
        :param password: Password for the archive (optional)
        :return: String representation of the archive contents
        """
        self._validate_archive_exists(archive)
        args = ["l", str(archive)]
        if password:
            args.append("-p" + password)
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
        Test the integrity of the archive
        :param archive: Path to the archive
        :param password: Password for the archive (optional)
        :return: Whether the test was successful
        """
        self._validate_archive_exists(archive)
        args = ["t", str(archive)]
        if password:
            args.append("-p" + password)
        logger.info(f"Testing integrity of {archive}")
        try:
            result = self._run_command(args)
            is_valid = "Everything is Ok" in result.stdout
            if is_valid:
                logger.success(f"Integrity test passed for {archive}")
            else:
                logger.warning(f"Integrity test failed for {archive}")
            return is_valid
        except SevenZipError as e:
            logger.error(f"Integrity test failed: {e}")
            raise SevenZipTestError("Integrity test failed") from e

    def execute(self, action: Action, files: Optional[List[Union[str, Path]]] = None,
                archive: Optional[Union[str, Path]] = None,
                destination: Optional[Union[str, Path]] = None,
                level: int = 5,
                password: Optional[str] = None,
                force: bool = False, **kwargs) -> Optional[str]:
        """
        Execute the corresponding 7z command based on the action type
        :param action: Action type ('compress', 'extract', 'list', 'test', 'delete', 'update')
        :param files: List of files to compress (only for 'compress' and 'update')
        :param archive: Path to the archive
        :param destination: Path to the extraction destination (only for 'extract')
        :param level: Compression level (only for 'compress')
        :param password: Password for the archive (optional)
        :param force: Whether to forcibly overwrite the destination directory (only for 'extract')
        :return: If the action is 'list', returns the archive contents; otherwise, no return value
        """
        logger.info(f"Executing action: {action}")
        try:
            match action:
                case 'compress':
                    if files is None or archive is None:
                        raise SevenZipValidationError(
                            "Compress action requires 'files' and 'archive' parameters")
                    self.compress(files, archive, level, password)
                case 'extract':
                    if archive is None or destination is None:
                        raise SevenZipValidationError(
                            "Extract action requires 'archive' and 'destination' parameters")
                    self.extract(archive, destination, password, force)
                case 'list':
                    if archive is None:
                        raise SevenZipValidationError(
                            "List action requires 'archive' parameter")
                    return self.list_contents(archive, password)
                case 'test':
                    if archive is None:
                        raise SevenZipValidationError(
                            "Test action requires 'archive' parameter")
                    return self.test_archive(archive, password)
                case 'delete':
                    if archive is None:
                        raise SevenZipValidationError(
                            "Delete action requires 'archive' parameter")
                    self.delete_archive(archive)
                case 'update':
                    if archive is None or files is None:
                        raise SevenZipValidationError(
                            "Update action requires 'archive' and 'files' parameters")
                    add = kwargs.get('add', True)
                    delete = kwargs.get('delete', False)
                    self.update_archive(archive, files, add, delete)
                case _:
                    raise SevenZipValidationError(f"Unsupported action type: {action}")
        except SevenZipError as e:
            logger.error(f"Action {action} failed: {e}")
            raise e

    def delete_archive(self, archive: Union[str, Path]) -> None:
        """
        Delete the specified archive
        :param archive: Path to the archive
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
        Update the archive by adding or deleting files
        :param archive: Path to the archive
        :param files: List of files to add or delete
        :param add: Whether to add files
        :param delete: Whether to delete files
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


def main():
    parser = argparse.ArgumentParser(description="7z Command-Line Tool Wrapper")
    subparsers = parser.add_subparsers(dest='action', help='Action types')

    # Compress
    compress_parser = subparsers.add_parser('compress', help='Compress files')
    compress_parser.add_argument(
        '-f', '--files', nargs='+', required=True, help='List of files to compress')
    compress_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    compress_parser.add_argument('-l', '--level', type=int, default=5, choices=range(0, 10),
                                 help='Compression level (0-9), default is 5')
    compress_parser.add_argument('-p', '--password', help='Password for the archive (optional)')

    # Extract
    extract_parser = subparsers.add_parser('extract', help='Extract files')
    extract_parser.add_argument('-a', '--archive', required=True, help='Path to the archive')
    extract_parser.add_argument(
        '-d', '--destination', required=True, help='Path to the extraction destination')
    extract_parser.add_argument('-p', '--password', help='Password for the archive (optional)')
    extract_parser.add_argument(
        '--force', action='store_true', help='Force overwrite if the destination directory exists')

    # List contents
    list_parser = subparsers.add_parser('list', help='List archive contents')
    list_parser.add_argument('-a', '--archive', required=True, help='Path to the archive')
    list_parser.add_argument('-p', '--password', help='Password for the archive (optional)')

    # Test integrity
    test_parser = subparsers.add_parser('test', help='Test archive integrity')
    test_parser.add_argument('-a', '--archive', required=True, help='Path to the archive')
    test_parser.add_argument('-p', '--password', help='Password for the archive (optional)')

    # Delete archive
    delete_parser = subparsers.add_parser('delete', help='Delete archive')
    delete_parser.add_argument('-a', '--archive', required=True, help='Path to the archive')

    # Update archive
    update_parser = subparsers.add_parser('update', help='Update archive')
    update_parser.add_argument('-a', '--archive', required=True, help='Path to the archive')
    update_parser.add_argument(
        '-f', '--files', nargs='+', required=True, help='List of files to add or delete')
    update_group = update_parser.add_mutually_exclusive_group(required=True)
    update_group.add_argument('--add', action='store_true', help='Add files')
    update_group.add_argument('--delete', action='store_true', help='Delete files')

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
                password=args.password
            )
        elif args.action == 'extract':
            zipper.execute(
                action='extract',
                archive=args.archive,
                destination=args.destination,
                password=args.password,
                force=args.force
            )
        elif args.action == 'list':
            content = zipper.execute(
                action='list',
                archive=args.archive,
                password=args.password
            )
            print("Archive Contents:\n", content)
        elif args.action == 'test':
            is_valid = zipper.execute(
                action='test',
                archive=args.archive,
                password=args.password
            )
            print(f"Is the archive valid? {is_valid}")
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
    except SevenZipError as e:
        logger.error(e)
        sys.exit(1)


if __name__ == "__main__":
    main()