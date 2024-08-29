import os
import requests
from threading import Thread, Lock
from queue import Queue
import logging
import argparse

# Global constants
DOWNLOAD_DIR = 'images'  # Directory to save downloaded images
# Base URL for image downloading
BASE_URL = "https://ngcicproject.observers.org/dss/n/{}/n{:04d}.jpg"
THREAD_COUNT = 10  # Default number of threads for concurrent downloading
RETRY_LIMIT = 3  # Maximum number of retry attempts for downloading

# Initialize a lock for logging to ensure thread-safe log access
log_lock = Lock()


def setup_logging():
    """
    Set up the logging configuration to log both to console and to a file.

    The log messages will include timestamps and the severity level.
    The logs will be saved in 'download.log' file, and
    will also be output to the console.
    """
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler("download.log"),  # Log to a file
            logging.StreamHandler()  # Log to console
        ]
    )


def download_image(image_id, retry_count=0):
    """
    Download an image from the server based on the image ID.

    Args:
        image_id (int): The ID of the image to download.
        retry_count (int): The current retry attempt count for this image.

    This function constructs the URL for the specified image ID,
    makes an HTTP GET request to download the image,
    and saves it to the specified directory.
    If the download fails, it will retry up to the specified limit
    before giving up and logging an error message.
    """
    prefix = image_id // 1000  # Calculate the prefix for the URL based on the image ID
    # Format the URL with the prefix and image ID
    url = BASE_URL.format(prefix, image_id)

    try:
        # Perform the HTTP request to download the image
        response = requests.get(url, timeout=10)
        if response.status_code == 200:  # Check for a successful response
            # Construct the filename
            filename = os.path.join(DOWNLOAD_DIR, f"n{image_id:04d}.jpg")
            with open(filename, 'wb') as file:  # Save the content to a file
                file.write(response.content)
            with log_lock:  # Acquire the lock for thread-safe logging
                logging.info(f"Downloaded: {filename}")
        else:
            raise requests.RequestException(
                f"Failed with status code: {response.status_code}")
    except requests.RequestException as e:
        if retry_count < RETRY_LIMIT:  # Check if we can retry
            with log_lock:
                logging.warning(f"Retry {retry_count + 1} for {url}: {e}")
            # Retry downloading the image
            download_image(image_id, retry_count + 1)
        else:
            with log_lock:
                logging.error(
                    f"Failed to download {url} after {RETRY_LIMIT} retries.")


def worker():
    """
    Worker function that processes the queue and downloads images.

    This function continually retrieves image IDs from the queue,
    calls the 'download_image' function to download the corresponding image,
    and marks the task as done once the download is complete.
    """
    while not queue.empty():  # Process until the queue is empty
        image_id = queue.get()  # Get the next image ID from the queue
        download_image(image_id)  # Download the image
        queue.task_done()  # Indicate that the task is completed


def create_download_dir():
    """
    Create the download directory if it does not exist.

    This function checks if the specified download directory exists,
    and if it doesn't, it creates the directory to ensure that
    there is a place to save the downloaded images.
    """
    if not os.path.exists(DOWNLOAD_DIR):
        os.makedirs(DOWNLOAD_DIR)  # Create the directory if it doesn't exist


def main(start, end):
    """
    Main function to start the download process.

    Args:
        start (int): The starting image ID (inclusive).
        end (int): The ending image ID (inclusive).

    This function fills the queue with image IDs, starts a number of worker threads
    to process the downloads concurrently, and waits for all tasks to complete before exiting.
    """
    # Fill the queue with image IDs
    for i in range(start, end + 1):
        queue.put(i)

    # Start worker threads
    threads = []
    for _ in range(THREAD_COUNT):
        # Create a new thread that runs the worker function
        thread = Thread(target=worker)
        thread.start()  # Start the thread
        threads.append(thread)  # Add the thread to the list

    # Wait for all tasks to be completed
    queue.join()  # Block until all items in the queue have been processed

    # Ensure all threads have finished execution
    for thread in threads:
        thread.join()  # Wait for each thread to finish


if __name__ == "__main__":
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(
        description="Download images with multi-threading.")
    parser.add_argument('--start', type=int, default=1,
                        help='Starting image ID (inclusive).')
    parser.add_argument('--end', type=int, default=7840,
                        help='Ending image ID (inclusive).')
    parser.add_argument('--threads', type=int, default=10,
                        help='Number of threads to use.')

    args = parser.parse_args()

    # Update thread count based on user input
    THREAD_COUNT = args.threads

    # Initialize task queue
    queue = Queue()

    # Set up logging
    setup_logging()

    # Create download directory if necessary
    create_download_dir()

    # Start the main download process
    main(args.start, args.end)
