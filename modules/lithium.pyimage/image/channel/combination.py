from pathlib import Path
from typing import List, Optional, Tuple
from PIL import Image
import numpy as np
from skimage import color
import cv2
from loguru import logger
import argparse
import sys
import concurrent.futures

# Configure Loguru logger
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO", format="{time} {level} {message}")


def resize_to_match(image: Image.Image, target_size: Tuple[int, int]) -> Image.Image:
    """Resize the image to match the target size."""
    logger.debug(f"Resizing image from {image.size} to {target_size}")
    return image.resize(target_size, Image.ANTIALIAS)


def load_image_as_gray(path: Path) -> Image.Image:
    """Load an image and convert it to grayscale."""
    logger.debug(f"Loading grayscale image: {path}")
    try:
        return Image.open(path).convert('L')
    except Exception as e:
        logger.error(f"Failed to load image {path}: {e}")
        raise


def combine_channels(channels: List[Image.Image], color_space: str = 'RGB') -> Image.Image:
    """Combine three channels into an image with the specified color space."""
    logger.info(f"Combining channels into {color_space} color space")
    color_space = color_space.upper()
    if color_space == 'RGB':
        return Image.merge("RGB", channels)
    elif color_space == 'LAB':
        lab_image = Image.merge("LAB", channels)
        return lab_image.convert('RGB')
    elif color_space == 'HSV':
        hsv_image = Image.merge("HSV", channels)
        return hsv_image.convert('RGB')
    elif color_space == 'HSI':
        hsi_array = np.dstack([np.array(ch) / 255.0 for ch in channels])
        rgb_array = color.hsv2rgb(hsi_array)  # Approximate HSI using HSV
        return Image.fromarray((rgb_array * 255).astype(np.uint8))
    elif color_space == 'YUV':
        yuv_image = Image.merge("YCbCr", channels)
        return yuv_image.convert('RGB')
    else:
        logger.error(f"Unsupported color space: {color_space}")
        raise ValueError(f"Unsupported color space: {color_space}")


def channel_combination(src_paths: List[Path], color_space: str = 'RGB') -> Image.Image:
    """Load, resize, and combine channel images."""
    if len(src_paths) != 3:
        logger.error(
            "Three source image paths are required to combine channels.")
        raise ValueError(
            "Three source image paths are required to combine channels.")

    logger.info(f"Starting channel combination into {color_space} color space")
    # Load images
    channels = [load_image_as_gray(path) for path in src_paths]

    # Resize to match the first image
    base_size = channels[0].size
    channels = [resize_to_match(ch, base_size) for ch in channels]

    # Combine channels
    combined_image = combine_channels(channels, color_space=color_space)
    logger.info("Channel combination completed")
    return combined_image


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Combine three channel images into a specified color space image.")
    parser.add_argument('color_space', type=str, choices=['RGB', 'LAB', 'HSV', 'HSI', 'YUV'],
                        help="Target color space.")
    parser.add_argument('src1', type=Path,
                        help="Path to the first channel image.")
    parser.add_argument('src2', type=Path,
                        help="Path to the second channel image.")
    parser.add_argument('src3', type=Path,
                        help="Path to the third channel image.")
    parser.add_argument('-o', '--output', type=Path, default=Path('./combined.png'),
                        help="Path to save the combined image.")
    parser.add_argument('-f', '--format', type=str, choices=['PNG', 'JPEG', 'BMP', 'TIFF'],
                        default='PNG', help="Format of the output image.")
    parser.add_argument('-b', '--batch', action='store_true',
                        help="Enable batch processing mode to process multiple image sets in the same directory.")
    parser.add_argument('-m', '--mapping', nargs=3, metavar=('CH1', 'CH2', 'CH3'),
                        help="Custom channel mapping (e.g., R G B).")
    return parser.parse_args()


def batch_process(directory: Path, color_space: str, output_dir: Path, img_format: str, mapping: Optional[List[str]] = None):
    """Batch process multiple sets of channel images in a directory."""
    logger.info(f"Starting batch processing in directory: {directory}")
    # Assume each image set consists of three files named <basename>_<channel>.<ext>
    channel_suffixes = mapping if mapping else ['R', 'G', 'B']
    r_images = sorted(directory.glob(f'*_{channel_suffixes[0]}.*'))

    def process_image_set(r_img: Path):
        basename = r_img.stem[:-len(f'_{channel_suffixes[0]}')]
        other_channels = [
            directory / f"{basename}_{channel}{r_img.suffix}" for channel in channel_suffixes[1:]]
        if not all(ch.exists() for ch in other_channels):
            logger.warning(f"Missing channel images for: {basename}")
            return
        try:
            combined = channel_combination(
                [r_img] + other_channels, color_space=color_space)
            output_path = output_dir / \
                f"{basename}_{color_space}.{img_format.lower()}"
            combined.save(output_path, format=img_format)
            logger.info(f"Saved combined image to {output_path}")
        except Exception as e:
            logger.error(f"Error processing {basename}: {e}")

    output_dir.mkdir(exist_ok=True)
    with concurrent.futures.ThreadPoolExecutor() as executor:
        executor.map(process_image_set, r_images)


def main():
    args = parse_args()

    if args.batch:
        # Batch processing mode
        src_dir = args.src1.parent  # Assume all images are in the same directory
        output_dir = Path('./batch_output')
        try:
            batch_process(src_dir, args.color_space,
                          output_dir, args.format, args.mapping)
        except Exception as e:
            logger.exception(f"Error during batch processing: {e}")
            sys.exit(1)
    else:
        # Single image set processing
        try:
            combined_image = channel_combination([args.src1, args.src2, args.src3],
                                                 color_space=args.color_space)
            combined_image.save(args.output, format=args.format)
            logger.info(
                f"Combined image saved to {args.output}, format: {args.format}")
        except Exception as e:
            logger.exception(f"Error during channel combination: {e}")
            sys.exit(1)


def library_combine_channels(src_paths: List[Path], color_space: str = 'RGB', output_path: Optional[Path] = None, img_format: str = 'PNG') -> Optional[Path]:
    """
    Library function to combine channels and optionally save the image.

    :param src_paths: List of three Paths to the channel images.
    :param color_space: Target color space.
    :param output_path: Path to save the combined image. If None, the image is not saved.
    :param img_format: Format to save the image.
    :return: Path to the saved image if output_path is provided, else None.
    """
    try:
        combined_image = channel_combination(
            src_paths, color_space=color_space)
        if output_path:
            combined_image.save(output_path, format=img_format)
            logger.info(
                f"Combined image saved to {output_path}, format: {img_format}")
            return output_path
        return None
    except Exception as e:
        logger.exception(f"Error in library_combine_channels: {e}")
        raise


if __name__ == "__main__":
    main()
