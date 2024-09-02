import cv2
import numpy as np

def read_image(image_path: str) -> np.ndarray:
    """
    Read an image from the given file path.

    Parameters:
        image_path (str): Path to the image file.

    Returns:
        np.ndarray: The image in a floating point format normalized to [0, 1].
    """
    image = cv2.imread(image_path, cv2.IMREAD_UNCHANGED).astype(np.float32) / 65535.0
    return image

def save_image(image: np.ndarray, output_path: str) -> None:
    """
    Save the image to the given file path.

    Parameters:
        image (np.ndarray): The image to save.
        output_path (str): Path to the output file.

    Returns:
        None
    """
    cv2.imwrite(output_path, (image * 65535).astype(np.uint16))
