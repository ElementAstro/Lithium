import subprocess
from pathlib import Path
from typing import Union, List, Optional
from loguru import logger
import shutil
import argparse
import sys


# Define custom exception classes
class UnzipError(Exception):
    """Base exception class for UnzipWrapper"""


class UnzipExtractionError(UnzipError):
    """Exception for extraction failures"""


class UnzipListError(UnzipError):
    """Exception for listing contents failures"""


class UnzipValidationError(UnzipError):
    """Exception for parameter validation failures"""


class UnzipIntegrityError(UnzipError):
    """Exception for archive integrity test failures"""


class UnzipDeleteError(UnzipError):
    """Exception for deletion failures"""


class UnzipWrapper:
    def __init__(self, executable: str = "unzip"):
        """
        Initialize the unzip command-line tool wrapper
        :param executable: Path to the unzip executable, defaults to "unzip" in the system PATH
        """
        self.executable = executable
        if not shutil.which(self.executable):
            logger.error(f"Executable not found: {self.executable}")
            raise UnzipValidationError(
                f"Executable does not exist: {self.executable}")
        logger.info(f"Using unzip executable: {self.executable}")

    def _run_command(self, args: List[str]) -> subprocess.CompletedProcess:
        """
        Run the unzip command and capture output
        :param args: Arguments to pass to unzip
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
            logger.error(f"unzip command failed with exit code {e.returncode}")
            logger.error(f"Error output: {e.stderr}")
            raise UnzipError(
                f"Command execution failed: {' '.join(command)}") from e

    def _validate_archive_exists(self, archive: Union[str, Path]) -> None:
        """
        Validate that the archive exists
        :param archive: Path to the archive
        """
        if not Path(archive).exists():
            logger.error(f"Archive does not exist: {archive}")
            raise UnzipValidationError(f"Archive does not exist: {archive}")
        logger.debug("Archive exists")

    def extract(self, archive: Union[str, Path], destination: Union[str, Path],
                password: Optional[str] = None, force: bool = False) -> None:
        """
        Extract the specified archive to the destination folder
        :param archive: Path to the archive
        :param destination: Path to the extraction destination
        :param password: Password for the archive (optional)
        :param force: Whether to forcibly overwrite the destination directory if it exists
        """
        logger.info(
            f"Starting extraction of {archive} to {destination} with force={force}")
        self._validate_archive_exists(archive)
        destination_path = Path(destination)

        if destination_path.exists():
            if force:
                logger.warning(
                    f"Destination directory exists and will be removed: {destination}")
                try:
                    shutil.rmtree(destination_path)
                    logger.info(
                        f"Removed destination directory: {destination}")
                except Exception as e:
                    logger.error(
                        f"Failed to remove destination directory: {e}")
                    raise UnzipExtractionError(
                        f"Failed to remove destination directory: {destination}") from e
            else:
                logger.error(f"Destination directory exists: {destination}")
                raise UnzipValidationError(
                    f"Destination directory exists: {destination}")

        try:
            destination_path.mkdir(parents=True, exist_ok=True)
            args = ["-o", str(archive), "-d", str(destination)]
            if password:
                args.extend(["-P", password])
            logger.debug(f"Unzip arguments: {args}")
            self._run_command(args)
            logger.success(
                f"Successfully extracted {archive} to {destination}")
        except UnzipError as e:
            logger.error(f"Extraction process failed: {e}")
            raise UnzipExtractionError("Extraction failed") from e

    def list_contents(self, archive: Union[str, Path],
                      password: Optional[str] = None) -> str:
        """
        List the contents of the archive
        :param archive: Path to the archive
        :param password: Password for the archive (optional)
        :return: String representation of the archive contents
        """
        logger.info(f"Listing contents of {archive}")
        self._validate_archive_exists(archive)
        args = ["-l", str(archive)]
        if password:
            args.extend(["-P", password])
        try:
            result = self._run_command(args)
            logger.success(f"Successfully listed contents of {archive}")
            return result.stdout
        except UnzipError as e:
            logger.error(f"Listing contents failed: {e}")
            raise UnzipListError("Listing contents failed") from e

    def test_integrity(self, archive: Union[str, Path],
                       password: Optional[str] = None) -> bool:
        """
        Test the integrity of the archive
        :param archive: Path to the archive
        :param password: Password for the archive (optional)
        :return: Whether the test was successful
        """
        logger.info(f"Testing integrity of {archive}")
        self._validate_archive_exists(archive)
        args = ["-t", str(archive)]
        if password:
            args.extend(["-P", password])
        try:
            result = self._run_command(args)
            is_valid = "No errors detected" in result.stderr or "Everything is Ok" in result.stdout
            if is_valid:
                logger.success(f"Integrity test passed for {archive}")
            else:
                logger.warning(f"Integrity test failed for {archive}")
            return is_valid
        except UnzipError as e:
            logger.error(f"Integrity test failed: {e}")
            raise UnzipIntegrityError("Integrity test failed") from e

    def delete_archive(self, archive: Union[str, Path]) -> None:
        """
        Delete the specified archive
        :param archive: Path to the archive
        """
        logger.info(f"Deleting archive: {archive}")
        self._validate_archive_exists(archive)
        try:
            Path(archive).unlink()
            logger.success(f"Successfully deleted archive: {archive}")
        except Exception as e:
            logger.error(f"Failed to delete archive: {e}")
            raise UnzipDeleteError(
                f"Failed to delete archive: {archive}") from e

    def update_archive(self, archive: Union[str, Path], files: List[Union[str, Path]],
                       add: bool = True, delete: bool = False) -> None:
        """
        Update the archive by adding or deleting files
        Note: unzip does not support updating archives. This method is a placeholder.
        :param archive: Path to the archive
        :param files: List of files to add or delete
        :param add: Whether to add files
        :param delete: Whether to delete files
        """
        logger.warning("Update operation is not supported by unzip.")
        raise NotImplementedError(
            "Update operation is not supported by unzip.")

    def execute(self, action: str, files: Optional[List[Union[str, Path]]] = None,
                archive: Optional[Union[str, Path]] = None,
                destination: Optional[Union[str, Path]] = None,
                password: Optional[str] = None,
                force: bool = False, **kwargs) -> Optional[str]:
        """
        Execute the corresponding unzip command based on the action type
        :param action: Action type ('extract', 'list', 'test', 'delete')
        :param files: List of files to add or delete (not supported)
        :param archive: Path to the archive
        :param destination: Path to the extraction destination (only for 'extract')
        :param password: Password for the archive (optional)
        :param force: Whether to forcibly overwrite the destination directory (only for 'extract')
        :return: If the action is 'list', returns the archive contents; if 'test', returns integrity status; otherwise, no return value
        """
        logger.info(f"Executing action: {action}")
        try:
            if action == 'extract':
                if archive is None or destination is None:
                    raise UnzipValidationError(
                        "Extract action requires 'archive' and 'destination' parameters")
                self.extract(archive, destination, password, force)
            elif action == 'list':
                if archive is None:
                    raise UnzipValidationError(
                        "List action requires 'archive' parameter")
                return self.list_contents(archive, password)
            elif action == 'test':
                if archive is None:
                    raise UnzipValidationError(
                        "Test action requires 'archive' parameter")
                return self.test_integrity(archive, password)
            elif action == 'delete':
                if archive is None:
                    raise UnzipValidationError(
                        "Delete action requires 'archive' parameter")
                self.delete_archive(archive)
            elif action == 'update':
                if archive is None or files is None:
                    raise UnzipValidationError(
                        "Update action requires 'archive' and 'files' parameters")
                add = kwargs.get('add', True)
                delete = kwargs.get('delete', False)
                self.update_archive(archive, files, add, delete)
            else:
                logger.error(f"Unsupported action type: {action}")
                raise UnzipValidationError(
                    f"Unsupported action type: {action}")
        except UnzipError as e:
            logger.error(f"Action {action} failed: {e}")
            raise e


def main():
    parser = argparse.ArgumentParser(
        description="Unzip Command-Line Tool Wrapper")
    subparsers = parser.add_subparsers(dest='action', help='Action types')

    # Extract action
    extract_parser = subparsers.add_parser('extract', help='Extract files')
    extract_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    extract_parser.add_argument(
        '-d', '--destination', required=True, help='Path to the extraction destination')
    extract_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')
    extract_parser.add_argument(
        '--force', action='store_true', help='Force overwrite if the destination directory exists')

    # List contents action
    list_parser = subparsers.add_parser('list', help='List archive contents')
    list_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    list_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')

    # Test integrity action
    test_parser = subparsers.add_parser(
        'test', help='Test archive integrity')
    test_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')
    test_parser.add_argument(
        '-p', '--password', help='Password for the archive (optional)')

    # Delete action
    delete_parser = subparsers.add_parser('delete', help='Delete archive')
    delete_parser.add_argument(
        '-a', '--archive', required=True, help='Path to the archive')

    args = parser.parse_args()

    if not args.action:
        parser.print_help()
        sys.exit(1)

    try:
        zipper = UnzipWrapper()
    except UnzipValidationError as e:
        logger.critical(e)
        sys.exit(1)

    try:
        if args.action == 'extract':
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
        else:
            logger.error(f"Unsupported action type: {args.action}")
            sys.exit(1)
    except UnzipError as e:
        logger.error(f"Action {args.action} failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
