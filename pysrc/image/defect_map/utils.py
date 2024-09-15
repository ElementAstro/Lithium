import cv2
import numpy as np

def save_image(filepath: str, image: np.ndarray) -> None:
    """
    Save an image to a specified file path.

    Parameters:
    - filepath: str, The file path where the image should be saved.
    - image: np.ndarray, The image data to be saved.
    """
    cv2.imwrite(filepath, image)

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
