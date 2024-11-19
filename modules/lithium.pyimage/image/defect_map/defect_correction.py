import numpy as np
from scipy.ndimage import generic_filter, median_filter, minimum_filter, maximum_filter, gaussian_filter
from skimage.filters import sobel
from skimage import img_as_float
from .interpolation import interpolate_defects
import multiprocessing
from typing import Optional, Tuple
from loguru import logger

# Configure Loguru logger
logger.add("defect_correction.log", rotation="10 MB", retention="10 days",
           level="DEBUG", format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}")


def defect_map_enhanced(image: np.ndarray, defect_map: np.ndarray, operation: str = 'mean', structure: str = 'square',
                        radius: int = 1, is_cfa: bool = False, protect_edges: bool = False,
                        adaptive_structure: bool = False) -> np.ndarray:
    """
    Enhanced Defect Map function to repair defective pixels in an image.

    Parameters:
    - image: np.ndarray, Input image to be repaired.
    - defect_map: np.ndarray, Defect map where defective pixels are marked as 0 (black).
    - operation: str, Operation to perform ('mean', 'gaussian', 'minimum', 'maximum', 'median', 'bilinear', 'bicubic').
    - structure: str, Neighborhood structure ('square', 'circular', 'horizontal', 'vertical').
    - radius: int, Radius or size of the neighborhood.
    - is_cfa: bool, If True, process the image as a CFA (Color Filter Array) image.
    - protect_edges: bool, Whether to protect the edges of the image.
    - adaptive_structure: bool, Whether to adaptively adjust the neighborhood based on defect density.

    Returns:
    - corrected_image: np.ndarray, The repaired image.
    """
    logger.info("Starting defect map enhancement")
    if structure == 'square':
        footprint = np.ones((2 * radius + 1, 2 * radius + 1))
    elif structure == 'circular':
        y, x = np.ogrid[-radius: radius + 1, -radius: radius + 1]
        footprint = x**2 + y**2 <= radius**2
    elif structure == 'horizontal':
        footprint = np.zeros((1, 2 * radius + 1))
        footprint[0, :] = 1
    elif structure == 'vertical':
        footprint = np.zeros((2 * radius + 1, 1))
        footprint[:, 0] = 1
    else:
        logger.error("Invalid structure type.")
        raise ValueError("Invalid structure type.")

    mask = defect_map == 0

    if protect_edges:
        edges = sobel(img_as_float(image))
        mask = np.logical_and(mask, edges < np.mean(edges))

    corrected_image = np.copy(image)

    if is_cfa:
        for c in range(3):  # Assuming an RGB image
            corrected_image[:, :, c] = correct_channel(
                image[:, :, c], mask[:, :, c], operation, footprint, adaptive_structure)
    else:
        corrected_image = correct_channel(
            image, mask, operation, footprint, adaptive_structure)

    logger.info("Defect map enhancement completed")
    return corrected_image


def correct_channel(channel: np.ndarray, mask: np.ndarray, operation: str, footprint: np.ndarray,
                    adaptive_structure: bool) -> np.ndarray:
    """
    Helper function to repair a single channel of an image.

    Parameters:
    - channel: np.ndarray, Image channel to be repaired.
    - mask: np.ndarray, Defect mask indicating defective pixels.
    - operation: str, Operation to perform.
    - footprint: np.ndarray, Neighborhood structure.
    - adaptive_structure: bool, Whether to adaptively adjust the neighborhood size.

    Returns:
    - channel: np.ndarray, The repaired image channel.
    """
    logger.debug(f"Correcting channel with operation: {operation}")
    if adaptive_structure:
        density = np.sum(mask) / mask.size
        radius = int(3 / density) if density > 0 else 1
        footprint = np.ones((2 * radius + 1, 2 * radius + 1))

    if operation == 'mean':
        channel_corrected = generic_filter(
            channel, np.mean, footprint=footprint, mode='constant', cval=np.nan)
    elif operation == 'gaussian':
        channel_corrected = gaussian_filter(channel, sigma=1)
    elif operation == 'minimum':
        channel_corrected = minimum_filter(channel, footprint=footprint)
    elif operation == 'maximum':
        channel_corrected = maximum_filter(channel, footprint=footprint)
    elif operation == 'median':
        channel_corrected = median_filter(channel, footprint=footprint)
    elif operation == 'bilinear':
        channel_corrected = interpolate_defects(channel, mask, method='linear')
    elif operation == 'bicubic':
        channel_corrected = interpolate_defects(channel, mask, method='cubic')
    else:
        logger.error("Invalid operation type.")
        raise ValueError("Invalid operation type.")

    channel[mask] = channel_corrected[mask]
    logger.debug("Channel correction completed")
    return channel


def parallel_defect_map(image: np.ndarray, defect_map: np.ndarray, **kwargs) -> np.ndarray:
    """
    Parallel processing for defect map repair.

    Parameters:
    - image: np.ndarray, Input image.
    - defect_map: np.ndarray, Defect map indicating defective pixels.
    - kwargs: Additional keyword arguments for defect_map_enhanced.

    Returns:
    - corrected_image: np.ndarray, The repaired image.
    """
    logger.info("Starting parallel defect map processing")
    if image.ndim == 2:
        return defect_map_enhanced(image, defect_map, **kwargs)

    pool = multiprocessing.Pool()
    channels = [image[:, :, i] for i in range(image.shape[2])]

    task_args = [(ch, defect_map, kwargs['operation'], kwargs['structure'], kwargs['radius'],
                  kwargs['is_cfa'], kwargs['protect_edges'], kwargs['adaptive_structure']) for ch in channels]

    results = pool.starmap(defect_map_enhanced_single_channel, task_args)
    pool.close()
    pool.join()

    logger.info("Parallel defect map processing completed")
    return np.stack(results, axis=-1)


def defect_map_enhanced_single_channel(channel: np.ndarray, defect_map: np.ndarray, operation: str,
                                       structure: str, radius: int, is_cfa: bool, protect_edges: bool,
                                       adaptive_structure: bool) -> np.ndarray:
    """
    Single-channel version of defect_map_enhanced for multiprocessing.

    Parameters:
    - channel: np.ndarray, Single image channel.
    - defect_map: np.ndarray, Defect map indicating defective pixels.
    - operation: str, Operation to perform.
    - structure: str, Neighborhood structure.
    - radius: int, Radius or size of the neighborhood.
    - is_cfa: bool, If True, process as a CFA image.
    - protect_edges: bool, Whether to protect the edges of the image.
    - adaptive_structure: bool, Whether to adaptively adjust the neighborhood size.

    Returns:
    - channel: np.ndarray, The repaired image channel.
    """
    logger.debug("Processing single channel for defect map enhancement")
    return defect_map_enhanced(channel, defect_map, operation, structure, radius, is_cfa, protect_edges, adaptive_structure)
