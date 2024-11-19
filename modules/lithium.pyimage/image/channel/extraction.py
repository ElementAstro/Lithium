import cv2
import numpy as np
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from matplotlib import pyplot as plt
from loguru import logger
import argparse
import sys
import concurrent.futures

# Configure Loguru logger
logger.remove()  # Remove the default logger
logger.add(sys.stderr, level="INFO", format="{time} {level} {message}")


def extract_channels(image: np.ndarray, color_space: str = 'RGB') -> Dict[str, np.ndarray]:
    """
    Extract channels from an image based on the specified color space.

    :param image: Input image in BGR format.
    :param color_space: Target color space for channel extraction.
    :return: Dictionary of channel names and their corresponding data.
    """
    channels = {}
    logger.debug(f"Extracting channels using color space: {color_space}")

    try:
        if color_space.upper() == 'RGB':
            rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            channels['R'], channels['G'], channels['B'] = cv2.split(rgb_image)

        elif color_space.upper() == 'XYZ':
            xyz_image = cv2.cvtColor(image, cv2.COLOR_BGR2XYZ)
            channels['X'], channels['Y'], channels['Z'] = cv2.split(xyz_image)

        elif color_space.upper() == 'LAB':
            lab_image = cv2.cvtColor(image, cv2.COLOR_BGR2Lab)
            channels['L*'], channels['a*'], channels['b*'] = cv2.split(
                lab_image)

        elif color_space.upper() == 'LCH':
            lab_image = cv2.cvtColor(image, cv2.COLOR_BGR2Lab)
            L, a, b = cv2.split(lab_image)
            H, C = cv2.cartToPolar(a.astype(np.float32), b.astype(np.float32))
            channels['L*'] = L
            channels['C*'] = C
            channels['H*'] = H

        elif color_space.upper() == 'HSV':
            hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            channels['H'], channels['S'], channels['V'] = cv2.split(hsv_image)

        elif color_space.upper() == 'HSI':
            hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            H, S, V = cv2.split(hsv_image)
            I = V.copy()
            channels['H'] = H
            channels['Si'] = S
            channels['I'] = I

        elif color_space.upper() == 'YUV':
            yuv_image = cv2.cvtColor(image, cv2.COLOR_BGR2YUV)
            channels['Y'], channels['U'], channels['V'] = cv2.split(yuv_image)

        elif color_space.upper() == 'YCBCR':
            ycbcr_image = cv2.cvtColor(image, cv2.COLOR_BGR2YCrCb)
            channels['Y'], channels['Cb'], channels['Cr'] = cv2.split(
                ycbcr_image)

        elif color_space.upper() == 'HSL':
            hsl_image = cv2.cvtColor(image, cv2.COLOR_BGR2HLS)
            channels['H'], channels['S'], channels['L'] = cv2.split(hsl_image)

        elif color_space.upper() == 'CMYK':
            # Approximate CMYK by converting to CMY and inferring K
            cmy_image = 255 - image
            C, M, Y = cv2.split(cmy_image)
            K = np.minimum(C, np.minimum(M, Y))
            channels['C'] = C
            channels['M'] = M
            channels['Y'] = Y
            channels['K'] = K

        else:
            logger.error(f"Unsupported color space: {color_space}")
            raise ValueError(f"Unsupported color space: {color_space}")

        logger.info(
            f"Successfully extracted channels for color space: {color_space}")
    except Exception as e:
        logger.error(f"Error extracting channels: {e}")
        raise

    return channels


def show_histogram(channel_data: np.ndarray, title: str = 'Channel Histogram') -> None:
    """
    Display the histogram of a single channel.

    :param channel_data: Numpy array of the channel data.
    :param title: Title of the histogram plot.
    """
    logger.debug(f"Displaying histogram for: {title}")
    plt.figure()
    plt.title(title)
    plt.xlabel('Pixel Value')
    plt.ylabel('Frequency')
    plt.hist(channel_data.ravel(), bins=256, range=[
             0, 256], color='gray', alpha=0.7)
    plt.grid(True)
    plt.show()


def merge_channels(channels: Dict[str, np.ndarray]) -> Optional[np.ndarray]:
    """
    Merge channels back into a single image.

    :param channels: Dictionary of channel names and their data.
    :return: Merged image or None if insufficient channels.
    """
    logger.debug("Merging channels into a single image.")
    merged_image = None
    channel_list = list(channels.values())

    try:
        if len(channel_list) >= 3:
            merged_image = cv2.merge(channel_list[:3])
        elif len(channel_list) == 2:
            merged_image = cv2.merge([
                channel_list[0],
                channel_list[1],
                np.zeros_like(channel_list[0], dtype=channel_list[0].dtype)
            ])
        elif len(channel_list) == 1:
            merged_image = channel_list[0]
        else:
            logger.warning("No channels to merge.")
    except Exception as e:
        logger.error(f"Error merging channels: {e}")
        raise

    if merged_image is not None:
        logger.info("Channels successfully merged.")
    else:
        logger.warning("Merged image is None.")

    return merged_image


def process_directory(input_dir: Path, output_dir: Path, color_space: str = 'RGB') -> None:
    """
    Process all images in a directory: extract channels, display histograms, and save channels.

    :param input_dir: Directory containing input images.
    :param output_dir: Directory to save extracted channels.
    :param color_space: Color space for channel extraction.
    """
    logger.info(
        f"Processing directory: {input_dir} with color space: {color_space}")

    if not input_dir.exists() or not input_dir.is_dir():
        logger.error(
            f"Input directory does not exist or is not a directory: {input_dir}")
        raise NotADirectoryError(
            f"Input directory does not exist or is not a directory: {input_dir}")

    output_dir.mkdir(parents=True, exist_ok=True)
    supported_extensions = ('.png', '.jpg', '.jpeg', '.bmp', '.tiff')

    for image_path in input_dir.iterdir():
        if image_path.suffix.lower() in supported_extensions:
            logger.info(f"Processing image: {image_path.name}")
            try:
                image = cv2.imread(str(image_path))
                if image is None:
                    logger.warning(f"Failed to read image: {image_path}")
                    continue

                extracted_channels = extract_channels(image, color_space)

                for channel_name, channel_data in extracted_channels.items():
                    save_path = output_dir / \
                        f"{image_path.stem}_{channel_name}.png"
                    cv2.imwrite(str(save_path), channel_data)
                    logger.info(f"Saved channel {channel_name} to {save_path}")

                    # Optionally display histogram
                    show_histogram(
                        channel_data, title=f"{image_path.stem} - {channel_name}")

            except Exception as e:
                logger.error(f"Error processing image {image_path.name}: {e}")


def save_channels(channels: Dict[str, np.ndarray], output_dir: Path, base_name: str = 'output') -> None:
    """
    Save extracted channels to the specified directory.

    :param channels: Dictionary of channel names and their data.
    :param output_dir: Directory to save the channels.
    :param base_name: Base name for the saved channel files.
    """
    logger.debug(
        f"Saving channels to directory: {output_dir} with base name: {base_name}")
    output_dir.mkdir(parents=True, exist_ok=True)

    for channel_name, channel_data in channels.items():
        filename = output_dir / f"{base_name}_{channel_name}.png"
        cv2.imwrite(str(filename), channel_data)
        logger.info(f"Saved channel {channel_name} to {filename}")


def display_image(title: str, image: np.ndarray) -> None:
    """
    Display an image using OpenCV.

    :param title: Window title.
    :param image: Image data in BGR format.
    """
    logger.debug(f"Displaying image: {title}")
    cv2.imshow(title, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.

    :return: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="Extract and process image channels in various color spaces."
    )
    parser.add_argument('mode', type=str, choices=['extract', 'merge', 'process_dir'],
                        help="Mode of operation: extract, merge, or process_dir.")
    parser.add_argument('--color_space', type=str, choices=['RGB', 'XYZ', 'LAB', 'LCH',
                                                            'HSV', 'HSI', 'YUV', 'YCBCR',
                                                            'HSL', 'CMYK'],
                        default='RGB', help="Target color space.")
    parser.add_argument('--input', type=Path, required=True,
                        help="Path to the input image or directory.")
    parser.add_argument('--output', type=Path, default=Path('./output_channels'),
                        help="Path to save the extracted channels or merged image.")
    parser.add_argument('--base_name', type=str, default='output',
                        help="Base name for saving merged image.")
    parser.add_argument('--format', type=str, choices=['PNG', 'JPEG', 'BMP', 'TIFF'],
                        default='PNG', help="Format for the output image.")
    parser.add_argument('--show', action='store_true',
                        help="Display the merged image.")
    return parser.parse_args()


def library_extract_channels(image: np.ndarray, color_space: str = 'RGB') -> Dict[str, np.ndarray]:
    """
    Library function to extract channels from an image.

    :param image: Input image in BGR format.
    :param color_space: Target color space.
    :return: Dictionary of channel names and their data.
    """
    return extract_channels(image, color_space)


def library_merge_channels(channels: Dict[str, np.ndarray]) -> Optional[np.ndarray]:
    """
    Library function to merge channels into a single image.

    :param channels: Dictionary of channel names and their data.
    :return: Merged image or None.
    """
    return merge_channels(channels)


def main():
    args = parse_args()

    if args.mode == 'extract':
        # Single image channel extraction
        logger.info(
            f"Extracting channels from image: {args.input} with color space: {args.color_space}")
        try:
            image = cv2.imread(str(args.input))
            if image is None:
                logger.error(f"Failed to read image: {args.input}")
                sys.exit(1)

            extracted_channels = extract_channels(image, args.color_space)

            for channel_name, channel_data in extracted_channels.items():
                save_path = args.output / \
                    f"{args.input.stem}_{channel_name}.png"
                args.output.mkdir(parents=True, exist_ok=True)
                cv2.imwrite(str(save_path), channel_data)
                logger.info(f"Saved channel {channel_name} to {save_path}")

                # Optionally display histogram
                show_histogram(
                    channel_data, title=f"{args.input.stem} - {channel_name}")

        except Exception as e:
            logger.exception(f"Error during channel extraction: {e}")
            sys.exit(1)

    elif args.mode == 'merge':
        # Merge channels into a single image
        logger.info(f"Merging channels from directory: {args.input}")
        try:
            if not args.input.is_dir():
                logger.error(
                    f"Input path must be a directory for merging: {args.input}")
                sys.exit(1)

            channels = {}
            for channel_file in args.input.iterdir():
                if channel_file.is_file() and channel_file.suffix.lower() == '.png':
                    parts = channel_file.stem.split('_')
                    if len(parts) < 2:
                        logger.warning(
                            f"Skipping file with unexpected naming: {channel_file.name}")
                        continue
                    channel_name = parts[-1]
                    channels[channel_name] = cv2.imread(
                        str(channel_file), cv2.IMREAD_GRAYSCALE)

            merged_image = merge_channels(channels)
            if merged_image is not None:
                merged_image_bgr = cv2.cvtColor(
                    merged_image, cv2.COLOR_RGB2BGR)
                output_path = args.output / \
                    f"{args.base_name}_merged.{args.format.lower()}"
                cv2.imwrite(str(output_path), merged_image_bgr)
                logger.info(f"Merged image saved to {output_path}")

                if args.show:
                    display_image("Merged Image", merged_image_bgr)
            else:
                logger.error(
                    "Merged image is None. Check if sufficient channels were provided.")

        except Exception as e:
            logger.exception(f"Error during channel merging: {e}")
            sys.exit(1)

    elif args.mode == 'process_dir':
        # Batch processing mode
        logger.info(
            f"Batch processing images in directory: {args.input} with color space: {args.color_space}")
        try:
            process_directory(args.input, args.output, args.color_space)
        except Exception as e:
            logger.exception(f"Error during batch processing: {e}")
            sys.exit(1)
    else:
        logger.error(f"Unsupported mode: {args.mode}")
        sys.exit(1)


if __name__ == "__main__":
    main()
