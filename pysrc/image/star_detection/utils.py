import cv2
import numpy as np

def save_image(filepath: str, image: np.ndarray) -> None:
    """
    Save an image to a specified file path.

    Parameters:
    - filepath: The file path where the image should be saved.
    - image: The image data to be saved.
    """
    cv2.imwrite(filepath, image)

def display_image(window_name: str, image: np.ndarray) -> None:
    """
    Display an image in a new window.

    Parameters:
    - window_name: The name of the window where the image will be displayed.
    - image: The image data to be displayed.
    """
    cv2.imshow(window_name, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
