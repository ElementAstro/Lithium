import cv2
import numpy as np
from astropy.io import fits
from typing import List, Tuple

def load_fits_image(file_path: str) -> np.ndarray:
    """
    Load a FITS image from the specified file path.

    Parameters:
    - file_path: Path to the FITS file.

    Returns:
    - Image data as a numpy array.
    """
    with fits.open(file_path) as hdul:
        image_data = hdul[0].data
    return image_data

def preprocess_fits_image(image_data: np.ndarray) -> np.ndarray:
    """
    Preprocess FITS image by normalizing to the 0-255 range.

    Parameters:
    - image_data: Raw image data from the FITS file.

    Returns:
    - Preprocessed image data as a numpy array.
    """
    image_data = np.nan_to_num(image_data)
    image_data = image_data.astype(np.float64)
    image_data -= np.min(image_data)
    image_data /= np.max(image_data)
    image_data *= 255
    return image_data.astype(np.uint8)

def load_image(file_path: str) -> np.ndarray:
    """
    Load an image from the specified file path. Supports FITS and standard image formats.

    Parameters:
    - file_path: Path to the image file.

    Returns:
    - Loaded image as a numpy array.
    """
    if file_path.endswith('.fits'):
        image_data = load_fits_image(file_path)
        if image_data.ndim == 2:
            return preprocess_fits_image(image_data)
        elif image_data.ndim == 3:
            channels = [preprocess_fits_image(image_data[..., i]) for i in range(image_data.shape[2])]
            return cv2.merge(channels)
    else:
        image = cv2.imread(file_path, cv2.IMREAD_UNCHANGED)
        if image is None:
            raise ValueError("Unable to load image file: {}".format(file_path))
        return image

def apply_median_filter(image: np.ndarray, config) -> np.ndarray:
    """
    Apply median filtering to the image.

    Parameters:
    - image: Input image.
    - config: Configuration object containing median filter settings.

    Returns:
    - Filtered image.
    """
    return cv2.medianBlur(image, config.median_filter_size)

def wavelet_transform(image: np.ndarray, levels: int) -> List[np.ndarray]:
    """
    Perform wavelet transform using a Laplacian pyramid.

    Parameters:
    - image: Input image.
    - levels: Number of levels in the wavelet transform.

    Returns:
    - List of wavelet transformed images at each level.
    """
    pyramid = []
    current_image = image.copy()
    for _ in range(levels):
        down = cv2.pyrDown(current_image)
        up = cv2.pyrUp(down, current_image.shape[:2])

        # Resize up to match the original image size
        up = cv2.resize(up, (current_image.shape[1], current_image.shape[0]))

        # Calculate the difference to get the detail layer
        layer = cv2.subtract(current_image, up)
        pyramid.append(layer)
        current_image = down

    pyramid.append(current_image)  # Add the final low-resolution image
    return pyramid

def inverse_wavelet_transform(pyramid: List[np.ndarray]) -> np.ndarray:
    """
    Reconstruct the image from its wavelet pyramid representation.

    Parameters:
    - pyramid: List of wavelet transformed images at each level.

    Returns:
    - Reconstructed image.
    """
    image = pyramid.pop()
    while pyramid:
        up = cv2.pyrUp(image, pyramid[-1].shape[:2])

        # Resize up to match the size of the current level
        up = cv2.resize(up, (pyramid[-1].shape[1], pyramid[-1].shape[0]))

        # Add the detail layer to reconstruct the image
        image = cv2.add(up, pyramid.pop())

    return image

def background_subtraction(image: np.ndarray, background: np.ndarray) -> np.ndarray:
    """
    Subtract the background from the image using the provided background image.

    Parameters:
    - image: Original image.
    - background: Background image to subtract.

    Returns:
    - Image with background subtracted.
    """
    # Resize the background to match the original image size
    background_resized = cv2.resize(background, (image.shape[1], image.shape[0]))

    # Subtract background and ensure no negative values
    result = cv2.subtract(image, background_resized)
    result[result < 0] = 0
    return result

def binarize(image: np.ndarray, config) -> np.ndarray:
    """
    Binarize the image using a fixed threshold.

    Parameters:
    - image: Input image.
    - config: Configuration object containing binarization settings.

    Returns:
    - Binarized image.
    """
    _, binary_image = cv2.threshold(image, config.binarization_threshold, 255, cv2.THRESH_BINARY)
    return binary_image

def detect_stars(binary_image: np.ndarray) -> Tuple[List[Tuple[int, int]], List[Tuple[Tuple[int, int], float, float]]]:
    """
    Detect stars in a binary image by finding contours.

    Parameters:
    - binary_image: Binarized image.

    Returns:
    - Tuple containing a list of star centers and a list of star properties (center, area, perimeter).
    """
    contours, _ = cv2.findContours(binary_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    star_centers = []
    star_properties = []

    for contour in contours:
        M = cv2.moments(contour)
        if M['m00'] != 0:
            center = (int(M['m10'] / M['m00']), int(M['m01'] / M['m00']))
            star_centers.append(center)
            area = cv2.contourArea(contour)
            perimeter = cv2.arcLength(contour, True)
            star_properties.append((center, area, perimeter))

    return star_centers, star_properties
