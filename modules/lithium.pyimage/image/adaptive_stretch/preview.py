import matplotlib.pyplot as plt
import cv2
import numpy as np
from typing import Optional, Tuple
from loguru import logger
from .stretch import AdaptiveStretch


logger.add("preview.log", rotation="1 MB")


def apply_real_time_preview(
    image: np.ndarray,
    noise_threshold: float = 1e-4,
    contrast_protection: Optional[float] = None,
    max_curve_points: int = 106,
    roi: Optional[Tuple[int, int, int, int]] = None
) -> np.ndarray:
    """
    Real-time preview by iteratively applying adaptive stretch transformation and displaying the result.

    :param image: Input image as a numpy array (grayscale or color).
    :param noise_threshold: Threshold to treat brightness differences as noise.
    :param contrast_protection: Optional contrast protection parameter.
    :param max_curve_points: Maximum number of points in the transformation curve.
    :param roi: Optional region of interest defined as a tuple (x, y, width, height).
    :return: Processed image.
    """
    logger.info("Starting real-time preview")
    adaptive_stretch = AdaptiveStretch(
        noise_threshold, contrast_protection, max_curve_points)
    preview_image = adaptive_stretch.stretch(image, roi)

    if len(preview_image.shape) == 3:
        # Convert the image from BGR to RGB color space for displaying with matplotlib
        preview_image = cv2.cvtColor(preview_image, cv2.COLOR_BGR2RGB)
        logger.debug("Converted image color space to RGB")
    else:
        logger.debug("Processing grayscale image")

    # Display the image using matplotlib
    plt.imshow(preview_image, cmap='gray' if len(
        preview_image.shape) == 2 else None)
    plt.title(
        f"Noise Threshold: {noise_threshold}, Contrast Protection: {contrast_protection}")
    plt.show()
    logger.info("Real-time preview completed")
    return preview_image


def save_preview(image: np.ndarray, path: str) -> None:
    """
    Save the preview image to the specified path.

    :param image: Image to be saved.
    :param path: Path to save the image file.
    """
    logger.info(f"Saving preview image to {path}")
    success = cv2.imwrite(path, image)
    if success:
        logger.info(f"Image successfully saved to {path}")
    else:
        logger.error(f"Failed to save image to {path}")


def load_and_preview(
    path: str,
    noise_threshold: float = 1e-4,
    contrast_protection: Optional[float] = None,
    max_curve_points: int = 106,
    roi: Optional[Tuple[int, int, int, int]] = None
) -> np.ndarray:
    """
    Load an image and apply real-time preview.

    :param path: Path to the image file.
    :param noise_threshold: Threshold to treat brightness differences as noise.
    :param contrast_protection: Optional contrast protection parameter.
    :param max_curve_points: Maximum number of points in the transformation curve.
    :param roi: Optional region of interest defined as a tuple (x, y, width, height).
    :return: Processed image.
    """
    logger.info(f"Loading image from {path}")
    image = cv2.imread(path, cv2.IMREAD_COLOR)
    if image is None:
        logger.error(f"Failed to load image from {path}")
        raise FileNotFoundError(f"Image file not found: {path}")
    logger.info(f"Image successfully loaded from {path}")
    return apply_real_time_preview(image, noise_threshold, contrast_protection, max_curve_points, roi)