#!/usr/bin/env python3

import argparse
import subprocess
import json
from datetime import datetime
import logging
import sys
import shutil
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed

try:
    from rich import print
    from rich.console import Console
    from rich.table import Table
    from rich.progress import Progress, BarColumn, TimeRemainingColumn, TextColumn
except ImportError:
    print("[red]Please install the 'rich' library: pip install rich[/red]")
    sys.exit(1)


# Initialize Rich Console
console = Console()

# Configure Logging
logger = logging.getLogger('PythonPackageUpdater')
logger.setLevel(logging.INFO)
log_formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
file_handler = logging.FileHandler('PythonPackageUpdater.log')
file_handler.setFormatter(log_formatter)
logger.addHandler(file_handler)


def check_pip():
    """Check if pip is available."""
    if shutil.which("pip") is None:
        console.print(
            "[red]pip not found. Please ensure Python is correctly installed and added to PATH.[/red]")
        logger.error("pip is not installed or not in PATH.")
        sys.exit(1)


def get_installed_packages(update_all):
    """
    Retrieve a list of installed or outdated packages.
    :param update_all: Whether to retrieve all installed packages
    :return: List of package names
    """
    try:
        if update_all:
            result = subprocess.run(
                ['pip', 'list', '--format=json'], capture_output=True, text=True, check=True)
        else:
            result = subprocess.run(
                ['pip', 'list', '--outdated', '--format=json'], capture_output=True, text=True, check=True)
        packages = json.loads(result.stdout)
        return [pkg['name'] for pkg in packages]
    except subprocess.CalledProcessError as e:
        console.print(f"[red]Failed to retrieve package list: {e}[/red]")
        logger.error(f"Failed to retrieve package list: {e}")
        sys.exit(1)
    except json.JSONDecodeError as e:
        console.print(f"[red]Error parsing package list: {e}[/red]")
        logger.error(f"Error parsing package list: {e}")
        sys.exit(1)


def update_package(package):
    """
    Update a single package.
    :param package: Package name
    :return: (Package name, Success/Failure)
    """
    try:
        subprocess.run(['pip', 'install', '--upgrade', package], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        logger.info(f"Package updated successfully: {package}")
        return (package, True)
    except subprocess.CalledProcessError:
        logger.error(f"Failed to update package: {package}")
        return (package, False)


def update_python_packages(excluded_packages, update_all, parallel, dry_run):
    """
    Update Python packages, excluding specified ones.
    :param excluded_packages: List of packages to exclude
    :param update_all: Whether to update all packages
    :param parallel: Whether to perform updates in parallel
    :param dry_run: Whether to perform a dry run without actual updates
    :return: (List of updated packages, List of failed packages)
    """
    packages = get_installed_packages(update_all)
    updated_packages = []
    failed_packages = []

    # Exclude packages
    packages_to_update = [pkg for pkg in packages if pkg.lower()
                          not in [ex.lower() for ex in excluded_packages]]
    excluded = set(packages) - set(packages_to_update)

    if excluded:
        console.print(
            "[yellow]The following packages will be skipped based on the exclusion list:[/yellow]")
        for pkg in excluded:
            console.print(f"- {pkg}")

    if dry_run:
        console.print(
            "[cyan]Dry run mode: No packages will be actually updated.[/cyan]")
        console.print(
            f"Total packages detected for update: {len(packages_to_update)}.")
        return updated_packages, failed_packages

    if not packages_to_update:
        console.print("[green]No packages need to be updated.[/green]")
        return updated_packages, failed_packages

    console.print(
        f"[bold]Starting update of {len(packages_to_update)} packages...[/bold]")

    if parallel:
        # Limit the number of threads
        max_workers = min(4, len(packages_to_update))
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            future_to_pkg = {executor.submit(
                update_package, pkg): pkg for pkg in packages_to_update}
            with Progress(
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                "[progress.percentage]{task.percentage:>3.0f}%",
                TimeRemainingColumn(),
                console=console,
            ) as progress:
                task = progress.add_task(
                    "Updating packages...", total=len(future_to_pkg))
                for future in as_completed(future_to_pkg):
                    pkg, success = future.result()
                    if success:
                        updated_packages.append(pkg)
                    else:
                        failed_packages.append(pkg)
                    progress.advance(task)
    else:
        with Progress(
            TextColumn("[progress.description]{task.description}"),
            BarColumn(),
            "[progress.percentage]{task.percentage:>3.0f}%",
            TimeRemainingColumn(),
            console=console,
        ) as progress:
            task = progress.add_task(
                "Updating packages...", total=len(packages_to_update))
            for pkg in packages_to_update:
                success = update_package(pkg)[1]
                if success:
                    updated_packages.append(pkg)
                else:
                    failed_packages.append(pkg)
                progress.advance(task)

    return updated_packages, failed_packages


def generate_report(excluded_packages, updated_packages, failed_packages):
    """
    Generate an update report.
    :param excluded_packages: List of excluded packages
    :param updated_packages: List of updated packages
    :param failed_packages: List of failed packages
    """
    report_file = f"PythonPackageUpdateReport_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    try:
        with open(report_file, 'w') as f:
            f.write("Python Package Update Report\n")
            f.write("============================\n\n")
            f.write(f"Date: {datetime.now()}\n\n")
            f.write("Excluded Packages:\n")
            f.write('\n'.join(excluded_packages) + '\n\n')
            f.write("Updated Packages:\n")
            f.write('\n'.join(updated_packages) + '\n\n')
            f.write("Failed Packages:\n")
            f.write('\n'.join(failed_packages) + '\n\n')
            f.write("Summary:\n")
            f.write("--------\n")
            f.write(
                f"Total packages processed: {len(updated_packages) + len(failed_packages)}\n")
            f.write(f"Successfully updated: {len(updated_packages)}\n")
            f.write(f"Failed to update: {len(failed_packages)}\n")
        console.print(f"[green]Update report saved to: {report_file}[/green]")
        logger.info(f"Generated update report: {report_file}")
    except Exception as e:
        console.print(f"[red]Failed to generate report: {e}[/red]")
        logger.error(f"Failed to generate report: {e}")


def display_summary(updated_packages, failed_packages):
    """
    Display an update summary.
    :param updated_packages: List of updated packages
    :param failed_packages: List of failed packages
    """
    table = Table(title="Update Summary")
    table.add_column("Status", style="cyan", no_wrap=True)
    table.add_column("Package Name", style="magenta")

    if updated_packages:
        for pkg in updated_packages:
            table.add_row("Success", pkg)
    if failed_packages:
        for pkg in failed_packages:
            table.add_row("Failure", pkg)

    console.print(table)
    console.print(
        f"[bold green]Total packages successfully updated: {len(updated_packages)}[/bold green]")
    if failed_packages:
        console.print(
            f"[bold red]Packages failed to update: {len(failed_packages)}[/bold red]")


def validate_requirements_file(file_path):
    """
    Validate the existence of a requirements.txt file.
    :param file_path: File path
    """
    path = Path(file_path)
    if not path.is_file():
        console.print(
            f"[red]The specified requirements file does not exist: {file_path}[/red]")
        logger.error(f"Requirements file does not exist: {file_path}")
        sys.exit(1)


def install_requirements(file_path):
    """
    Install packages listed in a requirements.txt file.
    :param file_path: Path to requirements.txt
    """
    try:
        subprocess.run(['pip', 'install', '-r', file_path], check=True)
        logger.info(
            f"Successfully installed packages from requirements file: {file_path}")
    except subprocess.CalledProcessError as e:
        console.print(
            f"[red]Failed to install packages from requirements file: {e}[/red]")
        logger.error(f"Failed to install packages from requirements file: {e}")
        sys.exit(1)


def main():
    check_pip()

    parser = argparse.ArgumentParser(
        description="Python Package Updater Tool",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    Update all packages and generate a report:
        python pip_updater.py --update-all --generate-report

    Update outdated packages, excluding specified packages:
        python pip_updater.py --exclude package1,package2

    Update outdated packages in parallel:
        python pip_updater.py --parallel

    Dry run mode, no actual updates:
        python pip_updater.py --dry-run

    Install packages from a requirements.txt file:
        python pip_updater.py --install-requirements requirements.txt
"""
    )
    parser.add_argument('--exclude', type=str,
                        help="Comma-separated list of package names to exclude from updating.")
    parser.add_argument('--update-all', action='store_true',
                        help="Update all installed packages, not just outdated ones.")
    parser.add_argument('--generate-report',
                        action='store_true', help="Generate a report of the update process.")
    parser.add_argument('--parallel', action='store_true',
                        help="Update packages in parallel.")
    parser.add_argument('--dry-run', action='store_true',
                        help="Dry run mode: do not actually update any packages.")
    parser.add_argument('--install-requirements', type=str,
                        help="Install packages from the specified requirements.txt file.")

    args = parser.parse_args()

    # Parse excluded packages
    excluded_packages = [pkg.strip()
                         for pkg in args.exclude.split(',')] if args.exclude else []

    if args.install_requirements:
        validate_requirements_file(args.install_requirements)
        install_requirements(args.install_requirements)
        sys.exit(0)

    # Update packages
    updated_packages, failed_packages = update_python_packages(
        excluded_packages=excluded_packages,
        update_all=args.update_all,
        parallel=args.parallel,
        dry_run=args.dry_run
    )

    # Display summary
    display_summary(updated_packages, failed_packages)

    # Generate report
    if args.generate_report:
        generate_report(excluded_packages, updated_packages, failed_packages)


if __name__ == "__main__":
    main()
