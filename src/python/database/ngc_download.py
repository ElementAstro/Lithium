import os
import requests
from threading import Thread, Lock
from queue import Queue
import logging
import argparse

# Global constants
DOWNLOAD_DIR = 'images'
BASE_URL = "https://ngcicproject.observers.org/dss/n/{}/n{:04d}.jpg"
THREAD_COUNT = 10
RETRY_LIMIT = 3

# Initialize a lock for logging
log_lock = Lock()


def setup_logging():
    """
    Set up the logging configuration to log both to console and to a file.
    """
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler("download.log"),
            logging.StreamHandler()
        ]
    )


def download_image(image_id, retry_count=0):
    """
    Download an image from the server based on the image ID.

    Args:
        image_id (int): The ID of the image to download.
        retry_count (int): The current retry attempt count.
    """
    prefix = image_id // 1000
    url = BASE_URL.format(prefix, image_id)

    try:
        response = requests.get(url, timeout=10)
        if response.status_code == 200:
            filename = os.path.join(DOWNLOAD_DIR, f"n{image_id:04d}.jpg")
            with open(filename, 'wb') as file:
                file.write(response.content)
            log_lock.acquire()
            logging.info(f"Downloaded: {filename}")
            log_lock.release()
        else:
            raise requests.RequestException(
                f"Failed with status code: {response.status_code}")
    except requests.RequestException as e:
        if retry_count < RETRY_LIMIT:
            log_lock.acquire()
            logging.warning(f"Retry {retry_count+1} for {url}: {e}")
            log_lock.release()
            download_image(image_id, retry_count + 1)
        else:
            log_lock.acquire()
            logging.error(
                f"Failed to download {url} after {RETRY_LIMIT} retries.")
            log_lock.release()


def worker():
    """
    Worker function that processes the queue and downloads images.
    """
    while not queue.empty():
        image_id = queue.get()
        download_image(image_id)
        queue.task_done()


def create_download_dir():
    """
    Create the download directory if it does not exist.
    """
    if not os.path.exists(DOWNLOAD_DIR):
        os.makedirs(DOWNLOAD_DIR)


def main(start, end):
    """
    Main function to start the download process.

    Args:
        start (int): The starting image ID.
        end (int): The ending image ID.
    """
    # Fill the queue with image IDs
    for i in range(start, end + 1):
        queue.put(i)

    # Start worker threads
    threads = []
    for _ in range(THREAD_COUNT):
        thread = Thread(target=worker)
        thread.start()
        threads.append(thread)

    # Wait for all tasks to be completed
    queue.join()

    # Ensure all threads have finished execution
    for thread in threads:
        thread.join()


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
