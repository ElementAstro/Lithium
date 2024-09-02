import cv2
import numpy as np
from .preprocessing import apply_median_filter, wavelet_transform, inverse_wavelet_transform, binarize, detect_stars, background_subtraction
from typing import List, Tuple

class StarDetectionConfig:
    """
    Configuration class for star detection settings.
    """
    def __init__(self,
                 median_filter_size: int = 3,
                 wavelet_levels: int = 4,
                 binarization_threshold: int = 30,
                 min_star_size: int = 10,
                 min_star_brightness: int = 20,
                 min_circularity: float = 0.7,
                 max_circularity: float = 1.3,
                 scales: List[float] = [1.0, 0.75, 0.5],
                 dbscan_eps: float = 10,
                 dbscan_min_samples: int = 2):
        self.median_filter_size = median_filter_size
        self.wavelet_levels = wavelet_levels
        self.binarization_threshold = binarization_threshold
        self.min_star_size = min_star_size
        self.min_star_brightness = min_star_brightness
        self.min_circularity = min_circularity
        self.max_circularity = max_circularity
        self.scales = scales
        self.dbscan_eps = dbscan_eps
        self.dbscan_min_samples = dbscan_min_samples


def multiscale_detect_stars(image: np.ndarray, config: StarDetectionConfig) -> List[Tuple[int, int]]:
    """
    Detect stars in an image using multiscale analysis.

    Parameters:
    - image: Grayscale input image as a numpy array.
    - config: Configuration object containing detection settings.

    Returns:
    - List of detected star positions as (x, y) tuples.
    """
    all_stars = []
    for scale in config.scales:
        resized_image = cv2.resize(image, None, fx=scale, fy=scale, interpolation=cv2.INTER_LINEAR)
        filtered_image = apply_median_filter(resized_image, config)
        pyramid = wavelet_transform(filtered_image, config.wavelet_levels)
        background = pyramid[-1]
        subtracted_image = background_subtraction(filtered_image, background)
        pyramid = pyramid[:2]
        processed_image = inverse_wavelet_transform(pyramid)
        binary_image = binarize(processed_image, config)
        _, star_properties = detect_stars(binary_image)
        filtered_stars = filter_stars(star_properties, binary_image, config)
        # Adjust star positions back to original scale
        filtered_stars = [(int(x / scale), int(y / scale)) for (x, y) in filtered_stars]
        all_stars.extend(filtered_stars)
    return all_stars

def filter_stars(star_properties: List[Tuple[Tuple[int, int], float, float]], binary_image: np.ndarray, config: StarDetectionConfig) -> List[Tuple[int, int]]:
    """
    Filter detected stars based on shape, size, and brightness.

    Parameters:
    - star_properties: List of tuples containing star properties (center, area, perimeter).
    - binary_image: Binary image used for star detection.
    - config: Configuration object containing filter settings.

    Returns:
    - List of filtered star positions as (x, y) tuples.
    """
    filtered_stars = []
    for (center, area, perimeter) in star_properties:
        circularity = (4 * np.pi * area) / (perimeter ** 2)
        mask = np.zeros_like(binary_image)
        cv2.circle(mask, center, 5, 255, -1)
        star_pixels = cv2.countNonZero(mask)
        brightness = np.mean(binary_image[mask == 255])
        if (star_pixels > config.min_star_size and
            brightness > config.min_star_brightness and
            config.min_circularity <= circularity <= config.max_circularity):
            filtered_stars.append(center)
    return filtered_stars
