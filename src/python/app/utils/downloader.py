import os
import requests
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Optional, Tuple, List
from urllib.parse import urlparse

class Downloader:
    """
    A class to download files using HTTP Range Requests to perform multi-threaded downloads.
    """
    def __init__(self, url: str, num_threads: int = 4, timeout: int = 10, chunk_size: int = 1024 * 1024, output_path: str = None):
        """
        Initializes the downloader with the provided parameters.

        Parameters:
            url (str): The URL of the file to download.
            num_threads (int): The number of threads to use for downloading the file.
            timeout (int): The timeout (in seconds) for network requests.
            chunk_size (int): The size of each chunk to download (in bytes).
            output_path (str): The path where the downloaded file will be saved.
        """
        self.url = url
        self.num_threads = num_threads
        self.timeout = timeout
        self.chunk_size = chunk_size
        self.file_size: Optional[int] = None
        self.output_path = output_path or os.path.basename(urlparse(url).path)

    def download(self) -> None:
        """
        Starts the file download using multiple threads.
        """
        self._prepare_file()
        ranges = self._split_range(self._fetch_file_size())

        with ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            futures = {executor.submit(self._download_chunk, start, end): (start, end) for start, end in ranges}
            for future in as_completed(futures):
                try:
                    future.result()  # Waits for the thread to finish and raises any exceptions encountered.
                except Exception as exc:
                    print(f"Chunk {futures[future]} failed with exception {exc}.")

    def _fetch_file_size(self) -> int:
        """
        Fetches the size of the file from the server using a HEAD request.

        Returns:
            int: The size of the file in bytes.
        """
        response = requests.head(self.url, timeout=self.timeout)
        response.raise_for_status()  # Raises an HTTPError for bad responses.
        self.file_size = int(response.headers.get("Content-Length", 0))
        return self.file_size

    def _prepare_file(self) -> None:
        """
        Prepares the file for chunked writing by creating an empty file if it doesn't exist.
        """
        with open(self.output_path, "wb") as f:
            pass  # Simply create an empty file to ensure it exists.

    def _download_chunk(self, start: int, end: int, max_retries: int = 5) -> None:
        """
        Downloads a chunk of the file from `start` to `end` using HTTP Range headers.

        Parameters:
            start (int): The starting byte of the chunk.
            end (int): The ending byte of the chunk.
            max_retries (int): Maximum number of retries for downloading the chunk.
        """
        headers = {"Range": f"bytes={start}-{end}"}
        attempts = 0

        while attempts < max_retries:
            try:
                response = requests.get(self.url, headers=headers, timeout=self.timeout, stream=True)
                response.raise_for_status()

                with open(self.output_path, "r+b") as f:
                    f.seek(start)
                    for chunk in response.iter_content(chunk_size=self.chunk_size):
                        f.write(chunk)
                return  # Exit after successful download
            except requests.RequestException as e:
                attempts += 1
                print(f"Attempt {attempts} failed for chunk {start}-{end}: {e}")

        print(f"Failed to download chunk after {max_retries} attempts.")

    def _split_range(self, file_size: int) -> List[Tuple[int, int]]:
        """
        Splits the file size into ranges for each thread.

        Parameters:
            file_size (int): The total size of the file in bytes.

        Returns:
            List[Tuple[int, int]]: A list of tuples where each tuple contains the start and end bytes of the range.
        """
        chunk_size = file_size // self.num_threads
        return [(i * chunk_size, min((i + 1) * chunk_size - 1, file_size - 1)) for i in range(self.num_threads)]

if __name__ == "__main__":
    url = "https://example.com/largefile.zip"
    downloader = Downloader(url, num_threads=8, timeout=5, output_path="downloaded_file.zip")
    downloader.download()
