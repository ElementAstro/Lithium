import cv2
import numpy as np
from typing import Optional, List, Tuple
from .utils import save_image, load_image


def auto_histogram(image: Optional[np.ndarray] = None, clip_shadow: float = 0.01, clip_highlight: float = 0.01,
                   target_median: int = 128, method: str = 'gamma', apply_clahe: bool = False,
                   clahe_clip_limit: float = 2.0, clahe_tile_grid_size: Tuple[int, int] = (8, 8),
                   apply_noise_reduction: bool = False, noise_reduction_method: str = 'median',
                   apply_sharpening: bool = False, sharpening_strength: float = 1.0,
                   batch_process: bool = False, file_list: Optional[List[str]] = None) -> Optional[List[np.ndarray]]:
    """
    Apply automated histogram transformations and enhancements to an image or a batch of images.

    Parameters:
    - image: Input image, grayscale or RGB.
    - clip_shadow: Percentage of shadow pixels to clip.
    - clip_highlight: Percentage of highlight pixels to clip.
    - target_median: Target median value for histogram stretching.
    - method: Stretching method ('gamma', 'logarithmic', 'mtf').
    - apply_clahe: Apply CLAHE (Contrast Limited Adaptive Histogram Equalization).
    - clahe_clip_limit: CLAHE clip limit.
    - clahe_tile_grid_size: CLAHE grid size.
    - apply_noise_reduction: Apply noise reduction.
    - noise_reduction_method: Noise reduction method ('median', 'gaussian').
    - apply_sharpening: Apply image sharpening.
    - sharpening_strength: Strength of sharpening.
    - batch_process: Enable batch processing mode.
    - file_list: List of file paths for batch processing.

    Returns:
    - Processed image or list of processed images.
    """
    def histogram_clipping(image: np.ndarray, clip_shadow: float, clip_highlight: float) -> np.ndarray:
        flat = image.flatten()
        low_val = np.percentile(flat, clip_shadow * 100)
        high_val = np.percentile(flat, 100 - clip_highlight * 100)
        return np.clip(image, low_val, high_val)

    def gamma_transformation(image: np.ndarray, target_median: int) -> np.ndarray:
        mean_val = np.median(image)
        gamma = np.log(target_median / 255.0) / np.log(mean_val / 255.0)
        return np.array(255 * (image / 255.0) ** gamma, dtype='uint8')

    def logarithmic_transformation(image: np.ndarray) -> np.ndarray:
        c = 255 / np.log(1 + np.max(image))
        return np.array(c * np.log(1 + image), dtype='uint8')

    def mtf_transformation(image: np.ndarray, target_median: int) -> np.ndarray:
        mean_val = np.median(image)
        mtf = target_median / mean_val
        return np.array(image * mtf, dtype='uint8')

    def apply_clahe_method(image: np.ndarray, clip_limit: float, tile_grid_size: Tuple[int, int]) -> np.ndarray:
        clahe = cv2.createCLAHE(clipLimit=clip_limit,
                                tileGridSize=tile_grid_size)
        if len(image.shape) == 2:
            return clahe.apply(image)
        else:
            return cv2.merge([clahe.apply(channel) for channel in cv2.split(image)])

    def noise_reduction(image: np.ndarray, method: str) -> np.ndarray:
        if method == 'median':
            return cv2.medianBlur(image, 3)
        elif method == 'gaussian':
            return cv2.GaussianBlur(image, (3, 3), 0)
        else:
            raise ValueError("Invalid noise reduction method specified.")

    def sharpen_image(image: np.ndarray, strength: float) -> np.ndarray:
        kernel = np.array([[-1, -1, -1], [-1, 9 + strength, -1], [-1, -1, -1]])
        return cv2.filter2D(image, -1, kernel)

    def process_single_image(image: np.ndarray) -> np.ndarray:
        if apply_noise_reduction:
            image = noise_reduction(image, noise_reduction_method)

        image = histogram_clipping(image, clip_shadow, clip_highlight)

        if method == 'gamma':
            image = gamma_transformation(image, target_median)
        elif method == 'logarithmic':
            image = logarithmic_transformation(image)
        elif method == 'mtf':
            image = mtf_transformation(image, target_median)
        else:
            raise ValueError("Invalid method specified.")

        if apply_clahe:
            image = apply_clahe_method(
                image, clahe_clip_limit, clahe_tile_grid_size)

        if apply_sharpening:
            image = sharpen_image(image, sharpening_strength)

        return image

    if batch_process:
        if file_list is None:
            raise ValueError(
                "File list cannot be None when batch processing is enabled.")
        processed_images = []
        for file_path in file_list:
            image = load_image(file_path, method != 'mtf')
            processed_image = process_single_image(image)
            processed_images.append(processed_image)
            save_image(f'processed_{file_path}', processed_image)
        return processed_images
    else:
        return process_single_image(image)
