import cv2
import numpy as np
from dataclasses import dataclass
from typing import Tuple


@dataclass
class ImageProcessor:
    """
    A class that provides various image processing functionalities.
    """

    @staticmethod
    def save_image(filepath: str, image: np.ndarray) -> None:
        """
        Save an image to a specified file path.

        Parameters:
        - filepath: str, The file path where the image should be saved.
        - image: np.ndarray, The image data to be saved.
        """
        cv2.imwrite(filepath, image)

    @staticmethod
    def display_image(window_name: str, image: np.ndarray) -> None:
        """
        Display an image in a new window.

        Parameters:
        - window_name: str, The name of the window where the image will be displayed.
        - image: np.ndarray, The image data to be displayed.
        """
        cv2.imshow(window_name, image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    @staticmethod
    def load_image(filepath: str, color_mode: int = cv2.IMREAD_COLOR) -> np.ndarray:
        """
        Load an image from a specified file path.

        Parameters:
        - filepath: str, The file path from where the image will be loaded.
        - color_mode: int, The color mode in which to load the image (default: cv2.IMREAD_COLOR).

        Returns:
        - np.ndarray, The loaded image data.
        """
        return cv2.imread(filepath, color_mode)

    @staticmethod
    def resize_image(image: np.ndarray, size: Tuple[int, int]) -> np.ndarray:
        """
        Resize an image to a new size.

        Parameters:
        - image: np.ndarray, The image data to be resized.
        - size: Tuple[int, int], The new size as (width, height).

        Returns:
        - np.ndarray, The resized image data.
        """
        return cv2.resize(image, size)

    @staticmethod
    def crop_image(image: np.ndarray, start: Tuple[int, int], size: Tuple[int, int]) -> np.ndarray:
        """
        Crop a region from an image.

        Parameters:
        - image: np.ndarray, The image data to be cropped.
        - start: Tuple[int, int], The top-left corner of the crop region as (x, y).
        - size: Tuple[int, int], The size of the crop region as (width, height).

        Returns:
        - np.ndarray, The cropped image data.
        """
        x, y = start
        w, h = size
        return image[y:y+h, x:x+w]


# Defining the public API of the module
__all__ = [
    'ImageProcessor'
]
