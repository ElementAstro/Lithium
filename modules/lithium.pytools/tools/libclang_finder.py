# python
import platform
import glob
import argparse
import asyncio
from pathlib import Path
from typing import List, Optional, Literal
from dataclasses import dataclass, field

from clang.cindex import Config
from loguru import logger
from rich.console import Console
from rich.table import Table
from rich.progress import Progress

console = Console()


@dataclass
class LibClangFinderConfig:
    """Configuration for LibClangFinder."""
    custom_path: Optional[Path] = None
    clear_cache: bool = False
    search_patterns: List[str] = field(default_factory=list)
    cache_file: Path = Path("libclang_path_cache.txt")
    log_file: Path = Path("libclang_finder.log")


class LibClangFinder:
    """Finder class to locate the libclang library."""

    def __init__(self, config: LibClangFinderConfig):
        self.config = config
        self.libclang_path: Optional[Path] = None
        self.setup_logging()
        logger.debug(f"LibClangFinder initialized with config: {self.config}")

    def setup_logging(self) -> None:
        """
        Configure Loguru for logging.

        Args:
            None
        """
        logger.remove()
        logger.add(
            self.config.log_file,
            rotation="1 MB",
            retention="10 days",
            level="DEBUG",
            format="{time} | {level} | {message}"
        )
        logger.debug("Logging is configured.")

    def clear_cache(self) -> None:
        """Clears the cached libclang path."""
        if self.config.cache_file.exists():
            self.config.cache_file.unlink()
            logger.info(f"Cleared cache file: {self.config.cache_file}")
            console.print(
                f"[green]Cleared cache file: {self.config.cache_file}[/green]")

    def cache_libclang_path(self, path: Path) -> None:
        """Caches the found libclang path."""
        self.config.cache_file.write_text(str(path))
        logger.info(f"Cached libclang path: {path}")
        console.print(f"[green]Cached libclang path: {path}[/green]")

    def load_cached_libclang_path(self) -> Optional[Path]:
        """Loads the cached libclang path if available."""
        if self.config.cache_file.exists():
            path = Path(self.config.cache_file.read_text().strip())
            if path.is_file():
                logger.info(f"Loaded cached libclang path: {path}")
                console.print(
                    f"[blue]Loaded cached libclang path: {path}[/blue]")
                return path
        logger.debug("No valid cached libclang path found.")
        console.print("[yellow]No valid cached libclang path found.[/yellow]")
        return None

    def find_libclang_linux(self) -> Optional[Path]:
        """Finds libclang on Linux systems."""
        possible_patterns = [
            '/usr/lib/llvm-*/lib/libclang.so*',
            '/usr/local/lib/llvm-*/lib/libclang.so*',
            '/usr/lib/x86_64-linux-gnu/libclang.so*',
            '/usr/local/lib/x86_64-linux-gnu/libclang.so*',
        ]
        logger.info("Searching for libclang on Linux...")
        console.print(
            "[bold green]Searching for libclang on Linux...[/bold green]")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def find_libclang_macos(self) -> Optional[Path]:
        """Finds libclang on macOS systems."""
        possible_patterns = [
            '/usr/local/opt/llvm/lib/libclang.dylib',
            '/usr/local/lib/libclang.dylib',
            '/Library/Developer/CommandLineTools/usr/lib/libclang.dylib',
            '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib',
        ]
        logger.info("Searching for libclang on macOS...")
        console.print(
            "[bold green]Searching for libclang on macOS...[/bold green]")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def find_libclang_windows(self) -> Optional[Path]:
        """Finds libclang on Windows systems."""
        possible_patterns = [
            'C:\\Program Files\\LLVM\\bin\\libclang.dll',
            'C:\\Program Files (x86)\\LLVM\\bin\\libclang.dll',
            'C:\\LLVM\\bin\\libclang.dll',
        ]
        logger.info("Searching for libclang on Windows...")
        console.print(
            "[bold green]Searching for libclang on Windows...[/bold green]")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def search_paths(self, patterns: List[str]) -> List[Path]:
        """Searches for libclang paths based on provided glob patterns."""
        all_patterns = patterns + self.config.search_patterns
        found_paths = []
        for pattern in all_patterns:
            matches = glob.glob(pattern)
            found_paths.extend(matches)
            logger.debug(
                f"Searching with pattern '{pattern}', found: {matches}")
        unique_paths = sorted(set(Path(p)
                              for p in found_paths if Path(p).is_file()))
        logger.debug(f"Total unique libclang paths found: {unique_paths}")
        console.print(
            f"[blue]Total unique libclang paths found: {len(unique_paths)}[/blue]")
        return unique_paths

    def select_libclang_path(self, paths: List[Path]) -> Optional[Path]:
        """Selects the most suitable libclang path from the list."""
        if paths:
            selected_path = paths[-1]
            logger.info(f"Selected libclang path: {selected_path}")
            console.print(
                f"[bold green]Selected libclang path: {selected_path}[/bold green]")
            return selected_path
        logger.error("No libclang library found.")
        console.print("[bold red]No libclang library found.[/bold red]")
        return None

    def get_libclang_path(self) -> Path:
        """Retrieves the libclang path, using cache or searching if necessary."""
        if self.config.clear_cache:
            self.clear_cache()

        if self.config.custom_path and self.config.custom_path.is_file():
            logger.info(
                f"Using custom libclang path: {self.config.custom_path}")
            console.print(
                f"[blue]Using custom libclang path: {self.config.custom_path}[/blue]")
            self.cache_libclang_path(self.config.custom_path)
            return self.config.custom_path

        cached_path = self.load_cached_libclang_path()
        if cached_path:
            return cached_path

        system = platform.system()
        logger.info(f"Detected operating system: {system}")
        console.print(
            f"[bold blue]Detected operating system: {system}[/bold blue]")
        find_method = {
            'Linux': self.find_libclang_linux,
            'Darwin': self.find_libclang_macos,
            'Windows': self.find_libclang_windows,
        }.get(system)

        if find_method:
            libclang_path = find_method()
            if libclang_path:
                self.cache_libclang_path(libclang_path)
                return libclang_path
        else:
            logger.error(f"Unsupported operating system: {system}")
            console.print(
                f"[bold red]Unsupported operating system: {system}[/bold red]")
            raise RuntimeError(f"Unsupported operating system: {system}")

        raise RuntimeError("libclang library not found.")

    def configure_clang(self) -> None:
        """Configures clang with the found libclang path."""
        libclang_path = self.get_libclang_path()
        logger.info(f"Setting libclang path to: {libclang_path}")
        console.print(
            f"[bold green]Setting libclang path to: {libclang_path}[/bold green]")
        Config.set_library_file(str(libclang_path))

    def list_libclang_versions(self) -> List[Path]:
        """Lists all available libclang versions on the system."""
        system = platform.system()
        logger.info(f"Listing all libclang versions on {system}")
        console.print(
            f"[bold blue]Listing all libclang versions on {system}[/bold blue]")
        find_method = {
            'Linux': self.find_libclang_linux,
            'Darwin': self.find_libclang_macos,
            'Windows': self.find_libclang_windows,
        }.get(system)

        if find_method:
            paths = find_method()
            if paths:
                logger.info(f"Available libclang libraries: {paths}")
                table = Table(title="Available libclang Libraries")
                table.add_column("Path", justify="left",
                                 style="cyan", no_wrap=True)
                for path in paths:
                    table.add_row(str(path))
                console.print(table)
                return paths
        else:
            logger.error(f"Unsupported operating system: {system}")
            console.print(
                f"[bold red]Unsupported operating system: {system}[/bold red]")
        return []


def parse_arguments() -> LibClangFinderConfig:
    """Parses command-line arguments and returns the configuration."""
    parser = argparse.ArgumentParser(description="libclang Path Finder")
    subparsers = parser.add_subparsers(dest='command', help='Sub-commands')

    # Sub-command: configure
    configure_parser = subparsers.add_parser(
        'configure', help='Configure libclang')
    configure_parser.add_argument(
        '--path', type=Path, help="Custom path to libclang library")
    configure_parser.add_argument(
        '--clear-cache', action='store_true', help="Clear cached libclang path")
    configure_parser.add_argument('--search-patterns', nargs='*',
                                  default=[], help="Additional glob patterns to search for libclang")
    configure_parser.add_argument('--cache-file', type=Path, default=Path(
        "libclang_path_cache.txt"), help="Path to the cache file")
    configure_parser.add_argument(
        '--log-file', type=Path, default=Path("libclang_finder.log"), help="Path to the log file")

    # Sub-command: list
    list_parser = subparsers.add_parser(
        'list', help='List available libclang versions')
    list_parser.add_argument('--cache-file', type=Path, default=Path(
        "libclang_path_cache.txt"), help="Path to the cache file")
    list_parser.add_argument(
        '--log-file', type=Path, default=Path("libclang_finder.log"), help="Path to the log file")

    args = parser.parse_args()

    if args.command == 'configure':
        return LibClangFinderConfig(
            custom_path=args.path,
            clear_cache=args.clear_cache,
            search_patterns=args.search_patterns,
            cache_file=args.cache_file,
            log_file=args.log_file,
        )
    elif args.command == 'list':
        return LibClangFinderConfig(
            cache_file=args.cache_file,
            log_file=args.log_file,
        )
    else:
        parser.print_help()
        exit(1)


def main() -> None:
    """Main entry point for the script."""
    config = parse_arguments()
    finder = LibClangFinder(config)

    if config.clear_cache:
        finder.clear_cache()

    if config.custom_path:
        try:
            finder.configure_clang()
            logger.info("libclang configured successfully.")
            console.print(
                "[bold green]libclang configured successfully.[/bold green]")
        except Exception as e:
            logger.error(f"Failed to configure libclang: {e}")
            console.print(
                f"[bold red]Failed to configure libclang: {e}[/bold red]")
    elif 'list' in config.__dict__ and config.cache_file:
        versions = finder.list_libclang_versions()
        if not versions:
            console.print("[yellow]No libclang versions found.[/yellow]")
    else:
        try:
            finder.configure_clang()
            logger.info("libclang configured successfully.")
            console.print(
                "[bold green]libclang configured successfully.[/bold green]")
        except Exception as e:
            logger.error(f"Failed to configure libclang: {e}")
            console.print(
                f"[bold red]Failed to configure libclang: {e}[/bold red]")


if __name__ == "__main__":
    main()
