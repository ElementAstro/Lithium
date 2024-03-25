# DownloadManager Class Documentation

## Description

The `DownloadManager` class is designed to manage download tasks, allowing users to add, remove, start, pause, resume tasks and get information about downloaded bytes.

```cpp
// Create a DownloadManager instance with a file to save task list
DownloadManager manager("task_list.txt");

// Add a new download task with high priority
manager.add_task("https://example.com/file.zip", "downloads/file.zip", 2);

// Start downloading tasks using default settings
manager.start();

// Pause a specific task by index
manager.pause_task(0);

// Resume the paused task
manager.resume_task(0);

// Get the number of downloaded bytes for a task
size_t downloaded_bytes = manager.get_downloaded_bytes(0);
```

## Constructor

### `DownloadManager(const std::string &task_file)`

Constructs a DownloadManager object with the specified task file path.

## Public Methods

### `void add_task(const std::string &url, const std::string &filepath, int priority = 0)`

Adds a new download task with the given URL, local file path, and optional priority.

### `bool remove_task(size_t index)`

Removes a task at the specified index from the task list. Returns true if successful.

### `void start(size_t thread_count = std::thread::hardware_concurrency(), size_t download_speed = 0)`

Starts downloading tasks with the specified number of threads and download speed limit.

### `void pause_task(size_t index)`

Pauses the task at the specified index.

### `void resume_task(size_t index)`

Resumes the paused task at the specified index.

### `size_t get_downloaded_bytes(size_t index)`

Returns the number of bytes downloaded for the task at the given index.

## Private Methods

### `std::optional<size_t> get_next_task_index()`

Returns the index of the next task to download or empty if the task queue is empty.

### `std::optional<DownloadTask> get_next_task()`

Returns the next task to download or empty if the task queue is empty.

### `void run(size_t download_speed)`

Starts the download threads with the specified download speed limit.

### `void download_task(DownloadTask &task, size_t download_speed)`

Downloads the specified task with the given download speed limit.

### `void save_task_list_to_file()`

Saves the current task list to the file specified during object construction.

## Member Variables

- `task_file_` : File path to save the task list.
- `tasks_` : List of download tasks.
- `task_queue_` : Priority queue for tasks based on priority.
- `mutex_` : Mutex for thread synchronization.
- `running_` : Atomic flag indicating if downloads are in progress.
- `start_time_` : Time point when download manager starts.

- The priority queue ensures tasks are processed based on priority level.
- Thread safety is maintained using mutex for accessing shared data.

```cpp
int main() {
    DownloadManager manager("task_list.txt");

    manager.add_task("https://example.com/file1.zip", "downloads/file1.zip", 2);
    manager.add_task("https://example.com/file2.zip", "downloads/file2.zip", 1);

    manager.start(2, 1024); // Start downloading with 2 threads and speed limit of 1KB/s

    manager.pause_task(0); // Pause the first task

    manager.resume_task(0); // Resume the first task

    size_t downloaded_bytes = manager.get_downloaded_bytes(1); // Get downloaded bytes for the second task

    return 0;
}
```
