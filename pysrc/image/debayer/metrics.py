import cv2
import numpy as np
from skimage.metrics import peak_signal_noise_ratio as psnr, structural_similarity as ssim

def evaluate_image_quality(rgb_image: np.ndarray) -> dict:
    """
    Evaluate the quality of the debayered image.
    """
    gray_image = cv2.cvtColor(rgb_image, cv2.COLOR_BGR2GRAY)
    laplacian_var = cv2.Laplacian(gray_image, cv2.CV_64F).var()

    mean_colors = cv2.mean(rgb_image)[:3]

    return {
        "sharpness": laplacian_var,
        "mean_red": mean_colors[2],
        "mean_green": mean_colors[1],
        "mean_blue": mean_colors[0]
    }

def calculate_image_quality(original: np.ndarray, processed: np.ndarray) -> Tuple[float, float]:
    """
    Calculate PSNR and SSIM between the original and processed images.
    """
    psnr_value = psnr(original, processed, data_range=processed.max() - processed.min())
    ssim_value = ssim(original, processed, multichannel=True)
    return psnr_value, ssim_value
