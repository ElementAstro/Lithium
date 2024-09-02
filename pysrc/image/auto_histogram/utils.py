import cv2
import numpy as np
from typing import Tuple, Union

def save_image(filepath: str, image: np.ndarray):
    """
    Save an image to the specified filepath.

    :param filepath: Path to save the image.
    :param image: Image data.
    """
    cv2.imwrite(filepath, image)

def load_image(filepath: str, grayscale: bool = False) -> np.ndarray:
    """
    Load an image from the specified filepath.

    :param filepath: Path to load the image from.
    :param grayscale: Load image as grayscale if True.
    :return: Loaded image.
    """
    flags = cv2.IMREAD_GRAYSCALE if grayscale else cv2.IMREAD_COLOR
    return cv2.imread(filepath, flags)
