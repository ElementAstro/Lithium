from .histogram import auto_histogram
from .utils import save_image, load_image
import os
from typing import List


def process_directory(directory: str, output_directory: str, method: str = 'gamma', **kwargs):
    """
    Process all images in a directory using the auto_histogram function.

    :param directory: Input directory containing images to process.
    :param output_directory: Directory to save processed images.
    :param method: Histogram stretching method ('gamma', 'logarithmic', 'mtf').
    :param kwargs: Additional parameters for auto_histogram.
    """
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    file_list = [os.path.join(directory, file) for file in os.listdir(
        directory) if file.endswith(('.jpg', '.png'))]

    processed_images = auto_histogram(
        None, method=method, batch_process=True, file_list=file_list, **kwargs)

    for file, image in zip(file_list, processed_images):
        output_path = os.path.join(
            output_directory, f'processed_{os.path.basename(file)}')
        save_image(output_path, image)
