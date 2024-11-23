# package.py

#!/usr/bin/env python3
"""
package.py

A comprehensive Python package manager for downloading, building, and installing software packages on Linux.
Supports renaming, listing, copying, deleting, moving, and more advanced features, with Rich library for beautified output.

Usage:
    python package.py {install|update|list|check-updates|export-config} [package names...] [--config ConfigFilePath] [--export-file ExportConfigFilePath] [--jobs NumberOfParallelTasks]

Commands:
    install          : Install specified packages or all packages defined in the configuration file.
    update           : Check and update all packages to the latest versions.
    list             : List all configured packages and their versions.
    check-updates    : Check if there are updates available for all packages.
    export-config    : Export the current configuration to a YAML file.

Options:
    --config         : Specify the configuration file path (default: packages.yaml).
    --export-file    : Specify the path to export the configuration file (default: exported_config.yaml).
    --jobs           : Specify the number of parallel build tasks (default: number of CPU cores).

Examples:
    Install packages:
        python package.py install package1 package2

    Update all packages:
        python package.py update

    List all packages:
        python package.py list

    Check for updates:
        python package.py check-updates

    Export configuration file:
        python package.py export-config --export-file my_config.yaml
"""

import argparse
import subprocess
import sys
import tarfile
import os
import hashlib
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
import requests
import yaml
import semver
from rich.console import Console
from rich.logging import RichHandler
from rich.progress import Progress, BarColumn, TextColumn, TimeElapsedColumn, SpinnerColumn
from rich.prompt import Prompt, Confirm
from rich.table import Table
import logging

# Configure Rich logging
logging.basicConfig(
    level=logging.INFO,
    format="%(message)s",
    datefmt="[%X]",
    handlers=[RichHandler()]
)
logger = logging.getLogger("package_manager")

console = Console()


@dataclass
class Source:
    """Represents the source of a package, typically a Git repository or custom URL."""
    url: str
    type: str  # 'github', 'gitlab', 'custom'


@dataclass
class Package:
    """Represents the metadata and configuration of a software package."""
    name: str
    sources: List[Source]
    version: str
    dependencies: List[str]  # List of package dependencies
    checksum: Optional[Dict[str, str]]  # e.g., {'md5': '...', 'sha256': '...'}
    build_args: Optional[Dict[str, str]]  # Build system parameters


class PackageManager:
    """Core class for managing package downloading, building, and installation."""

    def __init__(self, config_file: str = "packages.yaml"):
        self.config_file = config_file
        self.packages: Dict[str, Package] = {}
        # Define directories for cache, installation, and logs
        self.cache_dir = Path(os.getenv("CPP_PACKAGE_CACHE", "./cache"))
        self.install_dir = Path(os.getenv("CPP_PACKAGE_INSTALL", "./install"))
        self.log_dir = Path(os.getenv("CPP_PACKAGE_LOGS", "./logs"))
        self.config_version = "1.0"

    def load_config(self):
        """Load package configuration from a YAML file."""
        try:
            with open(self.config_file, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)

            # Validate configuration format
            if not isinstance(config, dict) or 'version' not in config or 'packages' not in config:
                raise ValueError("Invalid configuration format")

            if config['version'] != self.config_version:
                logger.warning(
                    f"Configuration file version mismatch. Expected: {self.config_version}, Found: {config.get('version')}")

            # Parse package information into objects
            for name, info in config['packages'].items():
                if not all(k in info for k in ('sources', 'version')):
                    raise ValueError(f"Package {name} is missing required fields")

                sources = [Source(url=s['url'], type=s['type']) for s in info['sources']]
                self.packages[name] = Package(
                    name=name,
                    sources=sources,
                    version=info['version'],
                    dependencies=info.get('dependencies', []),
                    checksum=info.get('checksum'),
                    build_args=info.get('build_args', {})
                )
            logger.info("[bold green]Configuration file loaded successfully.[/bold green]")
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            raise

    def download_package(self, package: Package) -> Optional[Path]:
        """Download the package archive from its sources."""
        cache_dir = self.cache_dir / package.name / package.version
        cache_dir.mkdir(parents=True, exist_ok=True)

        archive_path = cache_dir / f"{package.name}-{package.version}.tar.gz"
        if archive_path.exists():
            logger.info(f"Using cached version {package.name} {package.version}")
            return archive_path

        for source in package.sources:
            try:
                url = self.construct_download_url(source, package)
                with Progress(SpinnerColumn(), TextColumn("{task.description}"), BarColumn(),
                              "[progress.percentage]{task.percentage:>3.1f}%", TimeElapsedColumn(),
                              transient=True) as progress:
                    task = progress.add_task(
                        f"Downloading {package.name} …", total=None)
                    response = requests.get(url, stream=True, timeout=30)
                    if response.status_code == 200:
                        with open(archive_path, 'wb') as f:
                            for chunk in response.iter_content(chunk_size=8192):
                                if chunk:
                                    f.write(chunk)
                        progress.remove_task(task)

                        if package.checksum and not self.verify_checksum(archive_path, package.checksum):
                            logger.error(f"Checksum mismatch for {package.name}, trying next source.")
                            archive_path.unlink()
                            continue
                        logger.info(
                            f"[green]Download successful: {package.name} version {package.version} from {source.type}[/green]")
                        return archive_path
                    else:
                        logger.error(
                            f"Failed to download {package.name} from {source.url}, status code: {response.status_code}")
            except requests.RequestException as e:
                logger.error(f"Error downloading {package.name} from {source.url}: {e}")

        logger.error(f"[red]Failed to download {package.name} from all sources.[/red]")
        return None

    def construct_download_url(self, source: Source, package: Package) -> str:
        """Construct the URL to download the package's source code archive."""
        if source.type == 'github':
            return f"{source.url}/archive/refs/tags/v{package.version}.tar.gz"
        elif source.type == 'gitlab':
            return f"{source.url}/-/archive/v{package.version}/{package.name}-v{package.version}.tar.gz"
        else:  # custom
            return source.url.format(version=package.version)

    def verify_checksum(self, file_path: Path, checksums: Dict[str, str]) -> bool:
        """Verify the checksum of the downloaded file."""
        for algorithm, expected in checksums.items():
            if algorithm not in ['md5', 'sha256']:
                logger.warning(f"Unsupported checksum algorithm: {algorithm}")
                continue

            hash_obj = hashlib.new(algorithm)
            with open(file_path, "rb") as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    hash_obj.update(chunk)

            if hash_obj.hexdigest() != expected:
                logger.error(f"{algorithm.upper()} checksum mismatch: {file_path}")
                return False
        return True

    def extract_package(self, archive_path: Path, package: Package) -> Optional[Path]:
        """Extract the package archive to the specified directory."""
        extract_dir = self.cache_dir / package.name / package.version
        try:
            with tarfile.open(archive_path, "r:gz") as tar:
                tar.extractall(path=extract_dir)
            logger.info(f"[green]Extracted {package.name} to {extract_dir}[/green]")
            # Assume the extracted directory starts with the package name
            extracted_dirs = list(extract_dir.glob(f"{package.name}*"))
            return extracted_dirs[0] if extracted_dirs else None
        except (tarfile.TarError, OSError) as e:
            logger.error(f"Failed to extract {package.name}: {e}")
            return None

    def detect_build_system(self, source_dir: Path) -> str:
        """Detect the build system used by the package."""
        if (source_dir / 'CMakeLists.txt').exists():
            return 'cmake'
        elif (source_dir / 'Makefile').exists():
            return 'make'
        elif (source_dir / 'configure').exists():
            return 'autotools'
        else:
            logger.warning(f"Unrecognized build system in {source_dir}, defaulting to 'manual'.")
            return 'manual'

    def build_package(self, package: Package, source_dir: Optional[Path], jobs: int):
        """Build the package using the detected build system."""
        if source_dir is None:
            logger.error(f"Invalid source directory for {package.name}.")
            return

        build_system = self.detect_build_system(source_dir)
        build_dir = self.cache_dir / package.name / package.version / "build"
        build_dir.mkdir(parents=True, exist_ok=True)

        install_dir = self.install_dir / package.name / package.version
        install_dir.mkdir(parents=True, exist_ok=True)

        if build_system == 'cmake':
            cmake_command = [
                "cmake", str(source_dir),
                f"-DCMAKE_INSTALL_PREFIX={install_dir}",
                "-DCMAKE_BUILD_TYPE=Release"
            ] + [f"-D{key}={value}" for key, value in package.build_args.items()]

            build_commands = [
                cmake_command,
                ["cmake", "--build", ".", "--target", "install", f"-j{jobs}"]
            ]
            self.run_build_commands(build_commands, build_dir)
        elif build_system == 'make':
            build_commands = [
                ["make", f"-j{jobs}"],
                ["make", "install"]
            ]
            self.run_build_commands(build_commands, source_dir)
        elif build_system == 'autotools':
            build_commands = [
                ["./configure", f"--prefix={install_dir}"],
                ["make", f"-j{jobs}"],
                ["make", "install"]
            ]
            self.run_build_commands(build_commands, source_dir)
        else:
            logger.error(f"Cannot build {package.name}, unsupported build system.")

    def run_build_commands(self, commands: List[List[str]], cwd: Path):
        """Execute a list of build commands for the package."""
        try:
            for command in commands:
                logger.debug(f"Executing command: {' '.join(command)} in {cwd}")
                subprocess.run(command, cwd=cwd, check=True,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            logger.info("[green]Build succeeded.[/green]")
        except subprocess.CalledProcessError as e:
            logger.error(f"Build failed: {e}")
            raise

    def process_package(self, package: Package, processed: set, jobs: int):
        """Process (download, extract, build) a package and its dependencies."""
        if package.name in processed:
            return

        for dep_name in package.dependencies:
            dependency, dep_version = self.parse_dependency(dep_name)
            dep_package = self.packages.get(dependency)
            if dep_package:
                if dep_version and semver.compare(dep_package.version, dep_version) < 0:
                    logger.warning(
                        f"Dependency {dep_name} version mismatch. Required: {dep_version}, Found: {dep_package.version}")
                self.process_package(dep_package, processed, jobs)
            else:
                logger.warning(f"Dependency package {dep_name} for {package.name} not found")

        archive_path = self.download_package(package)
        if archive_path:
            source_dir = self.extract_package(archive_path, package)
            self.build_package(package, source_dir, jobs)
        processed.add(package.name)

    def parse_dependency(self, dependency: str) -> Tuple[str, Optional[str]]:
        """Parse a dependency string to extract its name and version."""
        if '==' in dependency:
            return tuple(dependency.split('=='))
        return (dependency, None)

    def run(self, packages_to_install: Optional[List[str]] = None, jobs: int = 1):
        """Run the package manager to install specified packages."""
        self.load_config()
        processed = set()
        with ThreadPoolExecutor(max_workers=jobs) as executor:
            if packages_to_install:
                futures = {executor.submit(self.process_package, self.packages[name], processed, jobs)
                           for name in packages_to_install if name in self.packages}
                not_found = [
                    name for name in packages_to_install if name not in self.packages]
                if not_found:
                    for name in not_found:
                        logger.warning(f"Package {name} not found in configuration.")
            else:
                futures = {executor.submit(self.process_package, package, processed, jobs)
                           for package in self.packages.values()}

            for future in as_completed(futures):
                try:
                    future.result()
                except Exception as e:
                    logger.error(f"Error processing package: {e}")

    def check_for_updates(self):
        """Check if any packages have updates available."""
        for name, package in self.packages.items():
            for source in package.sources:
                if source.type == 'github':
                    url = f"{source.url}/releases/latest"
                    try:
                        response = requests.get(url, timeout=10)
                        if response.status_code == 200:
                            latest_version = response.url.rstrip('/').split('/')[-1].lstrip('v')
                            if semver.compare(latest_version, package.version) > 0:
                                logger.info(
                                    f"[yellow]Update available: {name} {package.version} -> {latest_version}[/yellow]")
                            else:
                                logger.info(
                                    f"{name} is up to date ({package.version})")
                            break
                    except requests.RequestException as e:
                        logger.error(f"Error checking updates for {name}: {e}")
                else:
                    logger.warning(f"Unsupported source type for update check: {source.type}")

    def export_config(self, output_file: str):
        """Export the current package configuration to a YAML file."""
        config = {
            'version': self.config_version,
            'packages': {name: {
                'sources': [{'url': s.url, 'type': s.type} for s in package.sources],
                'version': package.version,
                'dependencies': package.dependencies,
                'checksum': package.checksum,
                'build_args': package.build_args
            } for name, package in self.packages.items()}
        }
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                yaml.dump(config, f)
            logger.info(f"Configuration exported to {output_file}")
        except Exception as e:
            logger.error(f"Failed to export configuration: {e}")

    def list_packages(self):
        """List all configured packages and their versions."""
        table = Table(title="Configured Packages", show_lines=True)
        table.add_column("Package Name", style="cyan", no_wrap=True)
        table.add_column("Version", style="magenta")
        for name, package in self.packages.items():
            table.add_row(name, package.version)
        console.print(table)


def parse_args():
    """Parse command-line arguments for the package manager."""
    parser = argparse.ArgumentParser(
        description="Advanced C++ Package Manager",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    Install packages:
        python package.py install package1 package2

    Update all packages:
        python package.py update

    List all packages:
        python package.py list

    Check for updates:
        python package.py check-updates

    Export configuration file:
        python package.py export-config --export-file my_config.yaml
        """
    )
    subparsers = parser.add_subparsers(
        title="Commands", dest="command", required=True)

    # Install command
    install_parser = subparsers.add_parser(
        "install", help="Install specified packages or all packages.")
    install_parser.add_argument(
        "packages", nargs="*", help="Names of packages to install. If empty, install all packages.")
    install_parser.add_argument(
        "--config", default="packages.yaml", help="Path to the configuration file (default: packages.yaml).")
    install_parser.add_argument(
        "--jobs", type=int, default=os.cpu_count(), help="Number of parallel build tasks (default: number of CPU cores).")

    # Update command
    update_parser = subparsers.add_parser(
        "update", help="Check and update all packages to the latest versions.")
    update_parser.add_argument(
        "packages", nargs="*", help="Names of packages to update. If empty, update all packages.")
    update_parser.add_argument(
        "--config", default="packages.yaml", help="Path to the configuration file (default: packages.yaml).")
    update_parser.add_argument(
        "--jobs", type=int, default=os.cpu_count(), help="Number of parallel build tasks (default: number of CPU cores).")

    # List command
    list_parser = subparsers.add_parser(
        "list", help="List all configured packages and their versions.")
    list_parser.add_argument(
        "--config", default="packages.yaml", help="Path to the configuration file (default: packages.yaml).")

    # Check updates command
    check_updates_parser = subparsers.add_parser(
        "check-updates", help="Check if there are updates available for all packages.")
    check_updates_parser.add_argument(
        "--config", default="packages.yaml", help="Path to the configuration file (default: packages.yaml).")

    # Export configuration command
    export_config_parser = subparsers.add_parser(
        "export-config", help="Export the current configuration to a YAML file.")
    export_config_parser.add_argument(
        "--config", default="packages.yaml", help="Path to the configuration file (default: packages.yaml).")
    export_config_parser.add_argument(
        "--export-file", default="exported_config.yaml", help="Path to export the configuration file (default: exported_config.yaml).")

    return parser.parse_args()


def main():
    """Main entry point for the package manager."""
    args = parse_args()
    manager = PackageManager(config_file=args.config)

    if args.command == "install":
        manager.run(
            packages_to_install=args.packages if args.packages else None, jobs=args.jobs)
    elif args.command == "update":
        manager.check_for_updates()
        if args.packages:
            manager.run(packages_to_install=args.packages, jobs=args.jobs)
        else:
            manager.run(packages_to_install=None, jobs=args.jobs)
    elif args.command == "list":
        manager.load_config()
        manager.list_packages()
    elif args.command == "check-updates":
        manager.check_for_updates()
    elif args.command == "export-config":
        manager.load_config()
        manager.export_config(args.export_file)
    else:
        logger.error("[red]Invalid command.[/red]")
        sys.exit(1)


if __name__ == "__main__":
    main()