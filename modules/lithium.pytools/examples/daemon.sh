# Example 1: Start the daemon with default settings
# This example starts the daemon process with the default configuration settings.
$ python3 daemon.py start

# Example 2: Start the daemon with a custom process name and script path
# This example starts the daemon process to monitor a custom script with a specified process name.
$ python3 daemon.py start --process_name my_process --script_path /path/to/my_script.py

# Example 3: Start the daemon with custom resource thresholds
# This example starts the daemon process with custom CPU and memory usage thresholds.
$ python3 daemon.py start --cpu_threshold 90.0 --memory_threshold 1024.0

# Example 4: Start the daemon with a custom restart interval and maximum restarts
# This example starts the daemon process with a custom restart interval and maximum number of restarts.
$ python3 daemon.py start --restart_interval 10 --max_restarts 5

# Example 5: Start the daemon with a custom monitoring interval
# This example starts the daemon process with a custom monitoring interval.
$ python3 daemon.py start --monitor_interval 10

# Example 6: Stop the daemon
# This example stops the currently running daemon process.
$ python3 daemon.py stop

# Example 7: Check the status of the daemon
# This example checks whether the daemon process is currently running.
$ python3 daemon.py status