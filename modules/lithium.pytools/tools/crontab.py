import os
import subprocess
import argparse
from loguru import logger
from rich.console import Console
from rich.table import Table
from typing import List

# Initialize the Rich console for enhanced terminal output
console = Console()


class CrontabManager:
    def __init__(self, crontab_path: str = "/tmp/current_crontab"):
        """
        Initialize the CrontabManager with the specified crontab file path.

        Args:
            crontab_path (str): The path to the temporary crontab file.
        """
        self.crontab_path = crontab_path
        self._setup_logger()
        self._load_crontab()

    def _setup_logger(self):
        """
        Set up the Loguru logger with a rotating log file.
        Logs will rotate when they reach 1 MB in size.
        """
        logger.add("crontab_manager.log", rotation="1 MB")
        logger.info("CrontabManager initialized.")

    def _run_command(self, command: str, capture_output: bool = False) -> str:
        """
        Execute a system command and handle errors.

        Args:
            command (str): The command to execute.
            capture_output (bool): Whether to capture and return the command's output.

        Returns:
            str: The output of the command if capture_output is True.

        Raises:
            subprocess.CalledProcessError: If the command execution fails.
        """
        try:
            result = subprocess.run(
                command,
                shell=True,
                check=True,
                capture_output=capture_output,
                text=True
            )
            if capture_output:
                return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            logger.error(f"Error executing command: {command}\n{e}")
            raise

    def _load_crontab(self):
        """
        Load the current user's crontab into the temporary crontab file.
        If the user has no crontab, create an empty temporary file.
        """
        self._run_command(
            f"crontab -l > {self.crontab_path} 2>/dev/null || touch {self.crontab_path}"
        )
        logger.info("Crontab loaded.")

    def list_jobs(self) -> List[str]:
        """
        Retrieve all cron jobs from the temporary crontab file.

        Returns:
            List[str]: A list of cron job entries.
        """
        if os.path.exists(self.crontab_path):
            with open(self.crontab_path, "r") as file:
                jobs = file.readlines()
            logger.info("Listed cron jobs.")
            return jobs
        logger.warning("Crontab file does not exist.")
        return []

    def add_job(self, schedule: str, command: str):
        """
        Add a new cron job to the temporary crontab file.

        Args:
            schedule (str): The cron schedule expression.
            command (str): The command to execute.
        """
        new_job = f"{schedule} {command}\n"
        with open(self.crontab_path, "a") as file:
            file.write(new_job)
        self._update_crontab()
        logger.info(f"Added job: {new_job.strip()}")

    def remove_job(self, command: str):
        """
        Remove a specific cron job that contains the given command.

        Args:
            command (str): The command identifier to remove from cron jobs.
        """
        jobs = self.list_jobs()
        with open(self.crontab_path, "w") as file:
            for job in jobs:
                if command not in job:
                    file.write(job)
        self._update_crontab()
        logger.info(f"Removed job containing command: {command}")

    def update_job(self, old_command: str, new_schedule: str, new_command: str):
        """
        Update an existing cron job by replacing the old command with a new schedule and command.

        Args:
            old_command (str): The command identifier of the job to update.
            new_schedule (str): The new cron schedule expression.
            new_command (str): The new command to execute.
        """
        jobs = self.list_jobs()
        updated = False
        with open(self.crontab_path, "w") as file:
            for job in jobs:
                if old_command in job:
                    file.write(f"{new_schedule} {new_command}\n")
                    updated = True
                    logger.info(f"Updated job: {new_schedule} {new_command}")
                else:
                    file.write(job)
        if updated:
            self._update_crontab()
        else:
            logger.warning(f"No job found with command: {old_command}")

    def clear_jobs(self):
        """
        Remove all cron jobs from the temporary crontab file.
        """
        with open(self.crontab_path, "w") as file:
            file.write("")
        self._update_crontab()
        logger.info("Cleared all cron jobs.")

    def _update_crontab(self):
        """
        Apply the changes made to the temporary crontab file to the user's actual crontab.
        """
        self._run_command(f"crontab {self.crontab_path}")
        logger.info("Crontab updated.")

    def job_exists(self, command: str) -> bool:
        """
        Check if a cron job containing the specified command exists.

        Args:
            command (str): The command identifier to search for.

        Returns:
            bool: True if the job exists, False otherwise.
        """
        jobs = self.list_jobs()
        exists = any(command in job for job in jobs)
        logger.info(f"Job existence for '{command}': {exists}")
        return exists

    def disable_job(self, command: str):
        """
        Disable a specific cron job by commenting it out in the temporary crontab file.

        Args:
            command (str): The command identifier of the job to disable.
        """
        jobs = self.list_jobs()
        with open(self.crontab_path, "w") as file:
            for job in jobs:
                if command in job and not job.startswith("#"):
                    file.write(f"#{job}")
                else:
                    file.write(job)
        self._update_crontab()
        logger.info(f"Disabled job containing command: {command}")

    def enable_job(self, command: str):
        """
        Enable a previously disabled cron job by uncommenting it in the temporary crontab file.

        Args:
            command (str): The command identifier of the job to enable.
        """
        jobs = self.list_jobs()
        with open(self.crontab_path, "w") as file:
            for job in jobs:
                if command in job and job.startswith("#"):
                    file.write(job.lstrip("#"))
                else:
                    file.write(job)
        self._update_crontab()
        logger.info(f"Enabled job containing command: {command}")

    def search_jobs(self, keyword: str) -> List[str]:
        """
        Search for cron jobs that contain the specified keyword.

        Args:
            keyword (str): The keyword to search for within cron jobs.

        Returns:
            List[str]: A list of cron jobs that match the keyword.
        """
        jobs = self.list_jobs()
        results = [job for job in jobs if keyword in job]
        logger.info(
            f"Searched jobs with keyword '{keyword}': {len(results)} found.")
        return results

    def export_jobs(self, file_path: str):
        """
        Export all current cron jobs to a specified file.

        Args:
            file_path (str): The path to the file where cron jobs will be exported.
        """
        with open(file_path, "w") as file:
            file.writelines(self.list_jobs())
        logger.info(f"Exported cron jobs to {file_path}")

    def import_jobs(self, file_path: str):
        """
        Import cron jobs from a specified file and append them to the current crontab.

        Args:
            file_path (str): The path to the file from which cron jobs will be imported.
        """
        if os.path.exists(file_path):
            with open(file_path, "r") as file:
                jobs = file.readlines()
            with open(self.crontab_path, "a") as crontab_file:
                crontab_file.writelines(jobs)
            self._update_crontab()
            logger.info(f"Imported cron jobs from {file_path}")
        else:
            logger.error(f"File {file_path} does not exist")

    def view_logs(self, log_path: str = "/var/log/syslog") -> str:
        """
        Retrieve and return cron job-related logs from the specified log file.

        Args:
            log_path (str): The path to the log file to search for cron logs.

        Returns:
            str: The extracted cron logs.
        """
        if os.path.exists(log_path):
            logs = self._run_command(
                f"grep CRON {log_path}", capture_output=True)
            logger.info(f"Viewed logs from {log_path}")
            return logs
        logger.error(f"Log file {log_path} does not exist")
        return ""

    def display_jobs(self):
        """
        Display all cron jobs in a formatted table using Rich.
        Disabled jobs are highlighted in red.
        """
        jobs = self.list_jobs()
        table = Table(title="Crontab Jobs")
        table.add_column("Schedule", style="cyan")
        table.add_column("Command", style="magenta")
        for job in jobs:
            if job.startswith("#"):
                # Display disabled jobs with a red "Disabled" label
                table.add_row("[red]Disabled[/red]", job.lstrip("#").strip())
            else:
                # Split the job into schedule and command parts
                parts = job.strip().split(None, 5)
                if len(parts) >= 6:
                    schedule = " ".join(parts[:5])
                    command = parts[5]
                else:
                    schedule = job.strip()
                    command = ""
                table.add_row(schedule, command)
        console.print(table)

    def __del__(self):
        """
        Destructor to clean up temporary files when the CrontabManager instance is deleted.
        Removes the temporary crontab file if it exists.
        """
        if os.path.exists(self.crontab_path):
            os.remove(self.crontab_path)
            logger.info("Temporary crontab file removed.")


def main():
    parser = argparse.ArgumentParser(
        description="Crontab Manager Command-Line Tool")
    subparsers = parser.add_subparsers(
        dest="command", help="Available commands")

    # Add command
    parser_add = subparsers.add_parser("add", help="Add a new cron job")
    parser_add.add_argument("schedule", type=str,
                            help="Cron schedule expression")
    parser_add.add_argument("command", type=str, help="Command to execute")

    # Remove command
    parser_remove = subparsers.add_parser("remove", help="Remove a cron job")
    parser_remove.add_argument(
        "command", type=str, help="Command identifier to remove")

    # List command
    parser_list = subparsers.add_parser("list", help="List all cron jobs")

    # Update command
    parser_update = subparsers.add_parser(
        "update", help="Update an existing cron job")
    parser_update.add_argument(
        "old_command", type=str, help="Old command identifier")
    parser_update.add_argument(
        "new_schedule", type=str, help="New cron schedule expression")
    parser_update.add_argument(
        "new_command", type=str, help="New command to execute")

    # Clear command
    parser_clear = subparsers.add_parser("clear", help="Clear all cron jobs")

    # Check existence command
    parser_exists = subparsers.add_parser(
        "exists", help="Check if a cron job exists")
    parser_exists.add_argument(
        "command", type=str, help="Command identifier to check")

    # Disable command
    parser_disable = subparsers.add_parser(
        "disable", help="Disable a cron job")
    parser_disable.add_argument(
        "command", type=str, help="Command identifier to disable")

    # Enable command
    parser_enable = subparsers.add_parser("enable", help="Enable a cron job")
    parser_enable.add_argument(
        "command", type=str, help="Command identifier to enable")

    # Search command
    parser_search = subparsers.add_parser(
        "search", help="Search cron jobs by keyword")
    parser_search.add_argument(
        "keyword", type=str, help="Keyword to search for")

    # Export command
    parser_export = subparsers.add_parser(
        "export", help="Export cron jobs to a file")
    parser_export.add_argument(
        "file_path", type=str, help="Path to export the cron jobs")

    # Import command
    parser_import = subparsers.add_parser(
        "import", help="Import cron jobs from a file")
    parser_import.add_argument(
        "file_path", type=str, help="Path to import the cron jobs from")

    # View logs command
    parser_logs = subparsers.add_parser("logs", help="View cron job logs")
    parser_logs.add_argument(
        "--log-path", type=str, default="/var/log/syslog",
        help="Path to the log file (default: /var/log/syslog)"
    )

    args = parser.parse_args()
    manager = CrontabManager()

    if args.command == "add":
        manager.add_job(args.schedule, args.command)
        console.print("Job added successfully.")
    elif args.command == "remove":
        manager.remove_job(args.command)
        console.print("Job removed successfully.")
    elif args.command == "list":
        manager.display_jobs()
    elif args.command == "update":
        manager.update_job(
            args.old_command, args.new_schedule, args.new_command)
        console.print("Job updated successfully.")
    elif args.command == "clear":
        manager.clear_jobs()
        console.print("All jobs cleared successfully.")
    elif args.command == "exists":
        exists = manager.job_exists(args.command)
        console.print(f"Job exists: {exists}")
    elif args.command == "disable":
        manager.disable_job(args.command)
        console.print("Job disabled successfully.")
    elif args.command == "enable":
        manager.enable_job(args.command)
        console.print("Job enabled successfully.")
    elif args.command == "search":
        results = manager.search_jobs(args.keyword)
        if results:
            for job in results:
                console.print(job.strip())
        else:
            console.print("No jobs found with the given keyword.")
    elif args.command == "export":
        manager.export_jobs(args.file_path)
        console.print(f"Jobs exported to {args.file_path} successfully.")
    elif args.command == "import":
        manager.import_jobs(args.file_path)
        console.print(f"Jobs imported from {args.file_path} successfully.")
    elif args.command == "logs":
        logs = manager.view_logs(args.log_path)
        if logs:
            console.print(logs)
        else:
            console.print("No logs found.")
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
