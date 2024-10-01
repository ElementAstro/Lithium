import argparse
import subprocess
import tarfile
import os
import hashlib
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass
import requests
import yaml
import semver
from loguru import logger
from tqdm import tqdm


@dataclass
class Source:
    """Represents a source for a package, typically a Git repository or custom URL."""
    url: str
    type: str  # 'github', 'gitlab', 'custom'


@dataclass
class Package:
    """Represents metadata and configuration for a software package."""
    name: str
    sources: List[Source]
    version: str
    dependencies: List[str]  # List of package dependencies
    checksum: Optional[Dict[str, str]]  # e.g., {'md5': '...', 'sha256': '...'}
    # Arguments passed to the build system
    build_args: Optional[Dict[str, str]]


class PackageManager:
    """Central class for managing the download, building, and installation of software packages."""

    def __init__(self, config_file: str = "packages.yaml"):
        self.config_file = config_file
        self.packages: Dict[str, Package] = {}
        # Define directories for caching, installation, and logs
        self.cache_dir = Path(os.getenv("CPP_PACKAGE_CACHE", "./cache"))
        self.install_dir = Path(os.getenv("CPP_PACKAGE_INSTALL", "./install"))
        self.log_dir = Path(os.getenv("CPP_PACKAGE_LOGS", "./logs"))
        self.config_version = "1.0"

    def load_config(self):
        """Loads the package configuration from a YAML file."""
        try:
            with open(self.config_file, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)

            # Validate the configuration format
            if not isinstance(config, dict) or 'version' not in config or 'packages' not in config:
                raise ValueError("Invalid configuration format")

            if config['version'] != self.config_version:
                logger.warning(f"Config file version mismatch. Expected {
                               self.config_version}, got {config.get('version')}")

            # Parse package information into objects
            for name, info in config['packages'].items():
                if not all(k in info for k in ('sources', 'version')):
                    raise ValueError(
                        f"Missing required fields for package {name}")

                sources = [Source(url=s['url'], type=s['type'])
                           for s in info['sources']]
                self.packages[name] = Package(
                    name=name,
                    sources=sources,
                    version=info['version'],
                    dependencies=info.get('dependencies', []),
                    checksum=info.get('checksum'),
                    build_args=info.get('build_args', {})
                )
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            raise

    def download_package(self, package: Package) -> Optional[Path]:
        """Downloads the package archive from its sources."""
        cache_dir = self.cache_dir / package.name / package.version
        cache_dir.mkdir(parents=True, exist_ok=True)

        archive_path = cache_dir / f"{package.name}-{package.version}.tar.gz"
        if archive_path.exists():
            logger.info(f"Using cached version of {
                        package.name} {package.version}")
            return archive_path

        for source in package.sources:
            try:
                url = self.construct_download_url(source, package)
                response = requests.get(url, stream=True, timeout=20)
                if response.status_code == 200:
                    with open(archive_path, 'wb') as f:
                        for chunk in tqdm(response.iter_content(chunk_size=8192), desc=f'Downloading {package.name}'):
                            f.write(chunk)

                    if package.checksum and not self.verify_checksum(archive_path, package.checksum):
                        logger.error(f"Checksum verification failed for {
                                     package.name}. Retrying other sources.")
                        archive_path.unlink()
                        continue
                    logger.info(f"Downloaded {package.name} version {
                                package.version} from {source.type}")
                    return archive_path
            except requests.RequestException as e:
                logger.error(f"Error downloading {
                             package.name} from {source.url}: {e}")

        logger.error(f"Failed to download {package.name} from all sources")
        return None

    def construct_download_url(self, source: Source, package: Package) -> str:
        """Constructs the appropriate URL to download the package source archive."""
        if source.type == 'github':
            return f"{source.url}/archive/refs/tags/v{package.version}.tar.gz"
        elif source.type == 'gitlab':
            return f"{source.url}/-/archive/v{package.version}/{package.name}-v{package.version}.tar.gz"
        else:  # custom
            return source.url.format(version=package.version)

    def verify_checksum(self, file_path: Path, checksums: Dict[str, str]) -> bool:
        """Verifies the checksum of the downloaded file."""
        for algorithm, expected in checksums.items():
            if algorithm not in ['md5', 'sha256']:
                logger.warning(f"Unsupported checksum algorithm: {algorithm}")
                continue

            hash_obj = hashlib.new(algorithm)
            with open(file_path, "rb") as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    hash_obj.update(chunk)

            if hash_obj.hexdigest() != expected:
                logger.error(
                    f"{algorithm.upper()} checksum mismatch for {file_path}")
                return False
        return True

    def extract_package(self, archive_path: Path, package: Package) -> Optional[Path]:
        """Extracts the package archive to a directory."""
        extract_dir = self.cache_dir / package.name / package.version
        try:
            with tarfile.open(archive_path, "r:gz") as tar:
                tar.extractall(path=extract_dir)
            logger.info(f"Extracted {package.name} to {extract_dir}")
            return next(extract_dir.glob(f"{package.name}*"), None)
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
            logger.warning(f"No recognized build system found in {
                           source_dir}. Defaulting to 'manual'.")
            return 'manual'

    def build_package(self, package: Package, source_dir: Optional[Path]):
        """Builds the package using the detected build system."""
        if source_dir is None:
            logger.error(f"Source directory for {package.name} is not valid")
            return

        build_system = self.detect_build_system(source_dir)
        build_dir = self.cache_dir / package.name / package.version / "build"
        build_dir.mkdir(parents=True, exist_ok=True)

        install_dir = self.install_dir / package.name / package.version

        if build_system == 'cmake':
            cmake_command = [
                "cmake", str(source_dir),
                f"-DCMAKE_INSTALL_PREFIX={install_dir}",
                "-DCMAKE_BUILD_TYPE=Release"
            ] + [f"-D{key}={value}" for key, value in package.build_args.items()]

            self.run_build_commands([cmake_command, [
                                    "cmake", "--build", ".", "--target", "install", f"-j{os.cpu_count()}"]], build_dir)
        elif build_system == 'make':
            self.run_build_commands(
                [["make"], ["make", "install"]], source_dir)
        elif build_system == 'autotools':
            self.run_build_commands([
                ["./configure", f"--prefix={install_dir}"],
                ["make"], ["make", "install"]
            ], source_dir)
        else:
            logger.error(f"Cannot build {
                         package.name} as no supported build system was found.")

    def run_build_commands(self, commands: List[List[str]], cwd: Path):
        """Executes a list of shell commands for building the package."""
        try:
            for command in commands:
                subprocess.run(command, cwd=cwd, check=True)
            logger.info("Build successful")
        except subprocess.CalledProcessError as e:
            logger.error(f"Build failed: {e}")

    def process_package(self, package: Package, processed: set):
        """Processes (downloads, extracts, builds) the package and its dependencies."""
        if package.name in processed:
            return

        for dep_name in package.dependencies:
            dependency, dep_version = self.parse_dependency(dep_name)
            dep_package = self.packages.get(dependency)
            if dep_package:
                if semver.compare(dep_package.version, dep_version) < 0:
                    logger.warning(f"Dependency {dep_name} version mismatch for {
                                   package.name}. Required: {dep_version}, found: {dep_package.version}")
                else:
                    self.process_package(dep_package, processed)
            else:
                logger.warning(
                    f"Dependency {dep_name} not found for {package.name}")

        archive_path = self.download_package(package)
        if archive_path:
            source_dir = self.extract_package(archive_path, package)
            self.build_package(package, source_dir)
        processed.add(package.name)

    def parse_dependency(self, dependency: str) -> (str, str):
        """Parses a dependency string to extract its name and version."""
        if '==' in dependency:
            return tuple(dependency.split('=='))
        return (dependency, '')

    def run(self, packages_to_install: Optional[List[str]] = None):
        """Runs the package manager to install the specified packages."""
        self.load_config()
        processed = set()
        with ThreadPoolExecutor(max_workers=os.cpu_count() or 1) as executor:
            if packages_to_install:
                futures = {executor.submit(self.process_package, self.packages[name], processed): name
                           for name in packages_to_install if name in self.packages}
            else:
                futures = {executor.submit(self.process_package, package, processed): package.name
                           for package in self.packages.values()}

            for future in as_completed(futures):
                try:
                    future.result()
                except Exception as e:
                    logger.error(f"Error processing package {
                                 futures[future]}: {e}")

    def check_for_updates(self):
        """Checks if newer versions are available for all packages."""
        for name, package in self.packages.items():
            for source in package.sources:
                if source.type == 'github':
                    url = f"{source.url}/releases/latest"
                    try:
                        response = requests.get(url, timeout=10)
                        if response.status_code == 200:
                            latest_version = response.url.split(
                                '/')[-1].lstrip('v')
                            if semver.compare(latest_version, package.version) > 0:
                                logger.info(f"Update available for {name}: {
                                            package.version} -> {latest_version}")
                            else:
                                logger.info(
                                    f"{name} is up to date ({package.version})")
                            break
                    except requests.RequestException as e:
                        logger.error(f"Error checking updates for {
                                     name} from {source.type}: {e}")
                else:
                    logger.warning(
                        f"Update check not supported for source type: {source.type}")

    def export_config(self, output_file: str):
        """Exports the current package configuration to a YAML file."""
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
        with open(output_file, 'w', encoding='utf-8') as f:
            yaml.dump(config, f)
        logger.info(f"Exported config to {output_file}")


def parse_args():
    """Parses command-line arguments for the package manager."""
    parser = argparse.ArgumentParser(
        description="Advanced C++ Package Manager")
    parser.add_argument("command", choices=["install", "update", "list", "check-updates", "export-config"],
                        help="Command to execute")
    parser.add_argument("packages", nargs="*",
                        help="Packages to install or update")
    parser.add_argument("--config", default="packages.yaml",
                        help="Path to the config file")
    parser.add_argument("--export-file", default="exported_config.yaml",
                        help="Path to export the config file")
    parser.add_argument("--jobs", type=int, default=os.cpu_count(),
                        help="Number of parallel jobs for building")
    return parser.parse_args()


def main():
    """Main entry point for the package manager."""
    args = parse_args()
    manager = PackageManager(config_file=args.config)

    if args.command == "install":
        manager.run(args.packages)
    elif args.command == "update":
        manager.check_for_updates()
        if args.packages:
            manager.run(args.packages)
    elif args.command == "list":
        manager.load_config()
        for name, package in manager.packages.items():
            print(f"{name} ({package.version})")
    elif args.command == "check-updates":
        manager.check_for_updates()
    elif args.command == "export-config":
        manager.load_config()
        manager.export_config(args.export_file)


if __name__ == "__main__":
    main()
