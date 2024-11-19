import cv2
import numpy as np
from pathlib import Path
from typing import Optional
from loguru import logger


def save_image(filepath: Union[str, Path], image: np.ndarray) -> None:
    """
    Save an image to the specified filepath.

    :param filepath: Path to save the image.
    :param image: Image data.
    """
    try:
        filepath = Path(filepath)
        filepath.parent.mkdir(parents=True, exist_ok=True)
        success = cv2.imwrite(str(filepath), image)
        if success:
            logger.info(f"Image saved successfully at {filepath}")
        else:
            logger.error(f"Failed to save image at {filepath}")
    except Exception as e:
        logger.error(
            f"Exception occurred while saving image at {filepath}: {e}")
        raise


def load_image(filepath: Union[str, Path], grayscale: bool = False) -> Optional[np.ndarray]:
    """
    Load an image from the specified filepath.

    :param filepath: Path to load the image from.
    :param grayscale: Load image as grayscale if True.
    :return: Loaded image or None if failed.
    """
    try:
        filepath = Path(filepath)
        if not filepath.exists():
            logger.error(f"File does not exist: {filepath}")
            return None
        flags = cv2.IMREAD_GRAYSCALE if grayscale else cv2.IMREAD_COLOR
        image = cv2.imread(str(filepath), flags)
        if image is not None:
            logger.info(f"Image loaded successfully from {filepath}")
        else:
            logger.error(f"Failed to load image from {filepath}")
        return image
    except Exception as e:
        logger.error(
            f"Exception occurred while loading image from {filepath}: {e}")
        return None
