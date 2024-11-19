import platform
import glob
import argparse
from pathlib import Path
from typing import List, Optional
from dataclasses import dataclass, field

from clang.cindex import Config
from loguru import logger


@dataclass
class LibClangFinderConfig:
    custom_path: Optional[Path] = None
    clear_cache: bool = False
    search_patterns: List[str] = field(default_factory=list)
    cache_file: Path = Path("libclang_path_cache.txt")
    log_file: Path = Path("libclang_finder.log")


class LibClangFinder:
    def __init__(self, config: LibClangFinderConfig):
        self.config = config
        self.libclang_path: Optional[Path] = None

        # Configure logging with loguru
        logger.remove()
        logger.add(
            self.config.log_file,
            rotation="1 MB",
            retention="10 days",
            level="DEBUG",
            format="{time} | {level} | {message}"
        )
        logger.debug(f"LibClangFinder initialized with config: {self.config}")

    def clear_cache(self):
        if self.config.cache_file.exists():
            self.config.cache_file.unlink()
            logger.info(f"Cleared cache file: {self.config.cache_file}")

    def cache_libclang_path(self, path: Path):
        self.config.cache_file.write_text(str(path))
        logger.info(f"Cached libclang path: {path}")

    def load_cached_libclang_path(self) -> Optional[Path]:
        if self.config.cache_file.exists():
            path = Path(self.config.cache_file.read_text().strip())
            if path.is_file():
                logger.info(f"Loaded cached libclang path: {path}")
                return path
        logger.debug("No valid cached libclang path found.")
        return None

    def find_libclang_linux(self) -> Optional[Path]:
        possible_patterns = [
            '/usr/lib/llvm-*/lib/libclang.so*',
            '/usr/local/lib/llvm-*/lib/libclang.so*',
            '/usr/lib/x86_64-linux-gnu/libclang.so*',
            '/usr/local/lib/x86_64-linux-gnu/libclang.so*',
        ]
        logger.info("Searching for libclang on Linux...")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def find_libclang_macos(self) -> Optional[Path]:
        possible_patterns = [
            '/usr/local/opt/llvm/lib/libclang.dylib',
            '/usr/local/lib/libclang.dylib',
            '/Library/Developer/CommandLineTools/usr/lib/libclang.dylib',
            '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib',
        ]
        logger.info("Searching for libclang on macOS...")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def find_libclang_windows(self) -> Optional[Path]:
        possible_patterns = [
            'C:\\Program Files\\LLVM\\bin\\libclang.dll',
            'C:\\Program Files (x86)\\LLVM\\bin\\libclang.dll',
            'C:\\LLVM\\bin\\libclang.dll',
        ]
        logger.info("Searching for libclang on Windows...")
        paths = self.search_paths(possible_patterns)
        return self.select_libclang_path(paths)

    def search_paths(self, patterns: List[str]) -> List[Path]:
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
        return unique_paths

    def select_libclang_path(self, paths: List[Path]) -> Optional[Path]:
        if paths:
            selected_path = paths[-1]
            logger.info(f"Selected libclang path: {selected_path}")
            return selected_path
        logger.error("No libclang library found.")
        return None

    def get_libclang_path(self) -> Path:
        if self.config.clear_cache:
            self.clear_cache()

        if self.config.custom_path and self.config.custom_path.is_file():
            logger.info(
                f"Using custom libclang path: {self.config.custom_path}")
            self.cache_libclang_path(self.config.custom_path)
            return self.config.custom_path

        cached_path = self.load_cached_libclang_path()
        if cached_path:
            return cached_path

        system = platform.system()
        logger.info(f"Detected operating system: {system}")
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
            raise RuntimeError(f"Unsupported operating system: {system}")

        raise RuntimeError("libclang library not found.")

    def configure_clang(self):
        libclang_path = self.get_libclang_path()
        logger.info(f"Setting libclang path to: {libclang_path}")
        Config.set_library_file(str(libclang_path))

    def list_libclang_versions(self) -> List[Path]:
        system = platform.system()
        logger.info(f"Listing all libclang versions on {system}")
        find_method = {
            'Linux': self.find_libclang_linux,
            'Darwin': self.find_libclang_macos,
            'Windows': self.find_libclang_windows,
        }.get(system)

        if find_method:
            paths = find_method()
            logger.info(f"Available libclang libraries: {paths}")
            return paths
        else:
            logger.error(f"Unsupported operating system: {system}")
            return []


def parse_arguments() -> LibClangFinderConfig:
    parser = argparse.ArgumentParser(description="libclang Path Finder")
    parser.add_argument('--path', type=Path,
                        help="Custom path to libclang library")
    parser.add_argument('--clear-cache', action='store_true',
                        help="Clear cached libclang path")
    parser.add_argument('--search-patterns', nargs='*', default=[],
                        help="Additional glob patterns to search for libclang")
    parser.add_argument('--cache-file', type=Path, default=Path(
        "libclang_path_cache.txt"), help="Path to the cache file")
    parser.add_argument('--log-file', type=Path,
                        default=Path("libclang_finder.log"), help="Path to the log file")
    args = parser.parse_args()
    return LibClangFinderConfig(
        custom_path=args.path,
        clear_cache=args.clear_cache,
        search_patterns=args.search_patterns,
        cache_file=args.cache_file,
        log_file=args.log_file,
    )


def main():
    config = parse_arguments()
    finder = LibClangFinder(config)
    try:
        finder.configure_clang()
        logger.info("libclang configured successfully.")
    except Exception as e:
        logger.error(f"Failed to configure libclang: {e}")


if __name__ == "__main__":
    main()
