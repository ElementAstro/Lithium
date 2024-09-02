import cv2
import numpy as np
from typing import Tuple

def select_roi(image: np.ndarray, x: int, y: int, width: int, height: int) -> np.ndarray:
    """
    Select a Region of Interest (ROI) from the image.

    Parameters:
        image (np.ndarray): The input image.
        x (int): X-coordinate of the top-left corner of the ROI.
        y (int): Y-coordinate of the top-left corner of the ROI.
        width (int): Width of the ROI.
        height (int): Height of the ROI.

    Returns:
        np.ndarray: The selected ROI.
    """
    roi = image[y:y+height, x:x+width]
    return roi
