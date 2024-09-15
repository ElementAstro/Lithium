from pathlib import Path
from typing import Tuple, Dict, Optional, List
import json
import numpy as np
import cv2
from astropy.io import fits
from scipy import ndimage


def debayer_image(img: np.ndarray, bayer_pattern: Optional[str] = None) -> np.ndarray:
    bayer_patterns = {
        "RGGB": cv2.COLOR_BAYER_RGGB2BGR,
        "GBRG": cv2.COLOR_BAYER_GBRG2BGR,
        "BGGR": cv2.COLOR_BAYER_BGGR2BGR,
        "GRBG": cv2.COLOR_BAYER_GRBG2BGR
    }
    return cv2.cvtColor(img, bayer_patterns.get(bayer_pattern, cv2.COLOR_BAYER_RGGB2BGR))


def resize_image(img: np.ndarray, target_size: int) -> np.ndarray:
    scale = min(target_size / max(img.shape[:2]), 1)
    if scale < 1:
        return cv2.resize(img, None, fx=scale, fy=scale, interpolation=cv2.INTER_AREA)
    return img


def normalize_image(img: np.ndarray) -> np.ndarray:
    if not np.allclose(img, img.astype(np.uint8)):
        img = cv2.normalize(img, None, alpha=0, beta=255,
                            norm_type=cv2.NORM_MINMAX)
    return img.astype(np.uint8)


def stretch_image(img: np.ndarray, is_color: bool) -> np.ndarray:
    if is_color:
        return ComputeAndStretch_ThreeChannels(img, True)
    return ComputeStretch_OneChannels(img, True)


def detect_stars(img: np.ndarray, remove_hotpixel: bool, remove_noise: bool, do_star_mark: bool, mark_img: Optional[np.ndarray] = None) -> Tuple[np.ndarray, float, float, Dict[str, float]]:
    return StarDetectAndHfr(img, remove_hotpixel, remove_noise, do_star_mark, down_sample_mean_std=True, mark_img=mark_img)

# New functions for enhanced image processing


def apply_gaussian_blur(img: np.ndarray, kernel_size: int = 5) -> np.ndarray:
    """Apply Gaussian blur to reduce noise."""
    return cv2.GaussianBlur(img, (kernel_size, kernel_size), 0)


def apply_unsharp_mask(img: np.ndarray, kernel_size: int = 5, amount: float = 1.5) -> np.ndarray:
    """Apply unsharp mask to enhance image details."""
    blurred = cv2.GaussianBlur(img, (kernel_size, kernel_size), 0)
    return cv2.addWeighted(img, amount + 1, blurred, -amount, 0)


def equalize_histogram(img: np.ndarray) -> np.ndarray:
    """Apply histogram equalization to improve contrast."""
    if len(img.shape) == 2:
        return cv2.equalizeHist(img)
    else:
        ycrcb = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
        ycrcb[:, :, 0] = cv2.equalizeHist(ycrcb[:, :, 0])
        return cv2.cvtColor(ycrcb, cv2.COLOR_YCrCb2BGR)


def remove_hot_pixels(img: np.ndarray, threshold: float = 3.0) -> np.ndarray:
    """Remove hot pixels using median filter and thresholding."""
    median = ndimage.median_filter(img, size=3)
    diff = np.abs(img - median)
    mask = diff > (threshold * np.std(diff))
    img[mask] = median[mask]
    return img


def adjust_gamma(img: np.ndarray, gamma: float = 1.0) -> np.ndarray:
    """Adjust image gamma."""
    inv_gamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** inv_gamma) *
                     255 for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(img, table)


def apply_clahe(img: np.ndarray, clip_limit: float = 2.0, tile_grid_size: Tuple[int, int] = (8, 8)) -> np.ndarray:
    """Apply Contrast Limited Adaptive Histogram Equalization (CLAHE)."""
    if len(img.shape) == 2:
        clahe = cv2.createCLAHE(clipLimit=clip_limit,
                                tileGridSize=tile_grid_size)
        return clahe.apply(img)
    else:
        lab = cv2.cvtColor(img, cv2.COLOR_BGR2LAB)
        l, a, b = cv2.split(lab)
        clahe = cv2.createCLAHE(clipLimit=clip_limit,
                                tileGridSize=tile_grid_size)
        cl = clahe.apply(l)
        limg = cv2.merge((cl, a, b))
        return cv2.cvtColor(limg, cv2.COLOR_LAB2BGR)


def denoise_image(img: np.ndarray, h: float = 10) -> np.ndarray:
    """Apply Non-Local Means Denoising."""
    return cv2.fastNlMeansDenoisingColored(img, None, h, h, 7, 21)


def enhance_image(img: np.ndarray, config: Dict[str, bool]) -> np.ndarray:
    """Apply a series of image enhancements based on the configuration."""
    if config.get('remove_hot_pixels', False):
        img = remove_hot_pixels(img)
    if config.get('denoise', False):
        img = denoise_image(img)
    if config.get('equalize_histogram', False):
        img = equalize_histogram(img)
    if config.get('apply_clahe', False):
        img = apply_clahe(img)
    if config.get('unsharp_mask', False):
        img = apply_unsharp_mask(img)
    if config.get('adjust_gamma', False):
        img = adjust_gamma(img, gamma=config.get('gamma', 1.0))
    return img


def process_image(filepath: Path, config: Dict[str, bool], resize_size: int = 2048) -> Tuple[Optional[np.ndarray], Dict[str, float]]:
    try:
        img, header = fits.getdata(filepath, header=True)
    except Exception:
        return None, {"star_count": -1, "average_hfr": -1, "max_star": -1, "min_star": -1, "average_star": -1}

    is_color = 'BAYERPAT' in header
    if is_color:
        img = debayer_image(img, header['BAYERPAT'])

    img = resize_image(img, resize_size)
    img = normalize_image(img)

    # Apply image enhancements
    img = enhance_image(img, config)

    if config['do_stretch']:
        img = stretch_image(img, is_color)

    if config['do_star_count']:
        img, star_count, avg_hfr, area_range = detect_stars(
            img, config['remove_hotpixel'], config['remove_noise'], config['do_star_mark'])
        return img, {
            "star_count": float(star_count),
            "average_hfr": avg_hfr,
            "max_star": area_range['max'],
            "min_star": area_range['min'],
            "average_star": area_range['average']
        }

    return img, {"star_count": -1, "average_hfr": -1, "max_star": -1, "min_star": -1, "average_star": -1}


def ImageStretchAndStarCount_Optim(filepath: Path, config: Dict[str, bool], resize_size: int = 2048,
                                   jpg_file: Optional[Path] = None, star_file: Optional[Path] = None) -> Tuple[Optional[np.ndarray], Dict[str, float]]:
    img, result = process_image(filepath, config, resize_size)

    if jpg_file and img is not None:
        cv2.imwrite(str(jpg_file), img)
    if star_file:
        with star_file.open('w') as f:
            json.dump(result, f)

    return img, result


def StreamingDebayerAndStretch(fits_data: bytearray, width: int, height: int, config: Dict[str, bool],
                               resize_size: int = 2048, bayer_type: Optional[str] = None) -> Optional[np.ndarray]:
    img = np.frombuffer(fits_data, dtype=np.uint16).reshape(height, width)

    if bayer_type:
        img = debayer_image(img, bayer_type)

    img = resize_image(img, resize_size)
    img = normalize_image(img)

    # Apply image enhancements
    img = enhance_image(img, config)

    if config.get('do_stretch', True):
        img = stretch_image(img, len(img.shape) == 3)

    return img


def StreamingDebayer(fits_data: bytearray, width: int, height: int, config: Dict[str, bool],
                     resize_size: int = 2048, bayer_type: Optional[str] = None) -> Optional[np.ndarray]:
    img = np.frombuffer(fits_data, dtype=np.uint16).reshape(height, width)

    if bayer_type:
        img = debayer_image(img, bayer_type)

    img = resize_image(img, resize_size)
    img = normalize_image(img)

    # Apply image enhancements
    img = enhance_image(img, config)

    return img
