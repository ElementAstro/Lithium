# Example 1: Add a new cron job
# This example adds a new cron job that runs a backup script every day at midnight.
$ python3 crontab.py add "0 0 * * *" "/path/to/backup.sh"

# Example 2: Remove a cron job
# This example removes a cron job that contains the command /path/to/backup.sh.
$ python3 crontab.py remove "/path/to/backup.sh"

# Example 3: List all cron jobs
# This example lists all the current cron jobs.
$ python3 crontab.py list

# Example 4: Update an existing cron job
# This example updates an existing cron job that contains the command /path/to/backup.sh to run at 1 AM every day.
$ python3 crontab.py update "/path/to/backup.sh" "0 1 * * *" "/path/to/backup.sh"

# Example 5: Clear all cron jobs
# This example clears all the current cron jobs.
$ python3 crontab.py clear

# Example 6: Check if a cron job exists
# This example checks if a cron job that contains the command /path/to/backup.sh exists.
$ python3 crontab.py exists "/path/to/backup.sh"

# Example 7: Disable a cron job
# This example disables a cron job that contains the command /path/to/backup.sh.
$ python3 crontab.py disable "/path/to/backup.sh"

# Example 8: Enable a cron job
# This example enables a previously disabled cron job that contains the command /path/to/backup.sh.
$ python3 crontab.py enable "/path/to/backup.sh"

# Example 9: Search cron jobs by keyword
# This example searches for cron jobs that contain the keyword backup.
$ python3 crontab.py search "backup"

# Example 10: Export cron jobs to a file
# This example exports all the current cron jobs to a file named cron_jobs.txt.
$ python3 crontab.py export "cron_jobs.txt"

# Example 11: Import cron jobs from a file
# This example imports cron jobs from a file named cron_jobs.txt.
$ python3 crontab.py import "cron_jobs.txt"

# Example 12: View cron job logs
# This example views cron job-related logs from the default log file (/var/log/syslog).
$ python3 crontab.py logs

# Example 13: View cron job logs from a specific log file
# This example views cron job-related logs from a specified log file (/var/log/cron.log).
$ python3 crontab.py logs --log-path "/var/log/cron.log"