import concurrent
import cv2
from loguru import logger
from matplotlib.path import Path
from PIL import Image
from typing import Optional, Tuple, Literal


def resample_image(input_image_path: str,
                   output_image_path: str,
                   width: Optional[int] = None,
                   height: Optional[int] = None,
                   scale: Optional[float] = None,
                   resolution: Optional[Tuple[int, int]] = None,
                   interpolation: Literal[cv2.INTER_LINEAR,
                                          cv2.INTER_CUBIC, cv2.INTER_NEAREST] = cv2.INTER_LINEAR,
                   preserve_aspect_ratio: bool = True,
                   crop_area: Optional[Tuple[int, int, int, int]] = None,
                   edge_detection: bool = False,
                   color_space: Literal['BGR', 'GRAY', 'HSV', 'RGB'] = 'BGR',
                   batch_mode: bool = False,
                   output_format: str = 'jpg',
                   ) -> None:
    """
    Resamples an image with given dimensions, scale, resolution, and additional processing options.

    :param input_image_path: Path to the input image (or directory in batch mode).
    :param output_image_path: Path to save the resampled image(s).
    :param width: Desired width in pixels.
    :param height: Desired height in pixels.
    :param scale: Scale factor for resizing (e.g., 0.5 for half size, 2.0 for double size).
    :param resolution: Tuple of horizontal and vertical resolution (in dpi).
    :param interpolation: Interpolation method (e.g., cv2.INTER_LINEAR, cv2.INTER_CUBIC, cv2.INTER_NEAREST).
    :param preserve_aspect_ratio: Whether to preserve the aspect ratio of the original image.
    :param crop_area: Tuple defining the crop area (x, y, w, h).
    :param edge_detection: Whether to apply edge detection before resizing.
    :param color_space: Color space to convert the image to (e.g., 'GRAY', 'HSV', 'RGB').
    :param add_watermark: Whether to add a watermark to the image.
    :param watermark_text: The text to be used as a watermark.
    :param watermark_position: Position tuple (x, y) for the watermark.
    :param watermark_opacity: Opacity level for the watermark (0.0 to 1.0).
    :param batch_mode: Whether to process multiple images in a directory.
    :param output_format: Output image format (e.g., 'jpg', 'png', 'tiff').
    :param brightness: Brightness adjustment factor (1.0 = no change).
    :param contrast: Contrast adjustment factor (1.0 = no change).
    :param sharpen: Whether to apply sharpening to the image.
    :param rotate_angle: Angle to rotate the image (in degrees).
    """
    logger.info(f"Starting resampling process for: {input_image_path}")

    def process_image(image_path: Path, output_path: Path):
        logger.debug(f"Processing image: {image_path}")
        img = cv2.imread(str(image_path))
        if img is None:
            logger.error(f"Cannot load image from {image_path}")
            raise ValueError(f"Cannot load image from {image_path}")

        original_height, original_width = img.shape[:2]
        logger.debug(
            f"Original dimensions: width={original_width}, height={original_height}")

        # Crop if needed
        if crop_area:
            x, y, w, h = crop_area
            logger.debug(f"Cropping image: x={x}, y={y}, w={w}, h={h}")
            img = img[y:y+h, x:x+w]

        # Edge detection
        if edge_detection:
            logger.debug("Applying edge detection.")
            img = cv2.Canny(img, 100, 200)

        # Convert color space if needed
        img = change_color_space(img, color_space)

        # Calculate new dimensions
        if scale:
            new_width = int(original_width * scale)
            new_height = int(original_height * scale)
            logger.debug(
                f"Scaling image by factor: {scale}, new dimensions: width={new_width}, height={new_height}")
        else:
            if width and height:
                new_width = width
                new_height = height
            elif width:
                new_width = width
                new_height = int(
                    (width / original_width) * original_height) if preserve_aspect_ratio else height
                logger.debug(
                    f"Setting width to {width}, calculated height: {new_height}")
            elif height:
                new_height = height
                new_width = int((height / original_height) *
                                original_width) if preserve_aspect_ratio else width
                logger.debug(
                    f"Setting height to {height}, calculated width: {new_width}")
            else:
                new_width, new_height = original_width, original_height
                logger.debug(
                    f"No scaling parameters provided, keeping original dimensions.")

        # Perform resizing
        logger.debug(
            f"Resizing image to width={new_width}, height={new_height} with interpolation={interpolation}")
        resized_img = cv2.resize(
            img, (new_width, new_height), interpolation=interpolation)

        # Convert back to BGR if color_space was changed to RGB for saving with OpenCV
        if color_space == 'RGB' and len(resized_img.shape) == 3:
            logger.debug("Converting image from RGB back to BGR for saving.")
            resized_img = cv2.cvtColor(resized_img, cv2.COLOR_RGB2BGR)
        elif color_space == 'HSV' and len(resized_img.shape) == 3:
            logger.debug("Converting image from HSV back to BGR for saving.")
            resized_img = cv2.cvtColor(resized_img, cv2.COLOR_HSV2BGR)

        # Save the image with specified resolution if provided
        if resolution:
            logger.debug(f"Saving image with resolution: {resolution}")
            pil_img = Image.fromarray(cv2.cvtColor(resized_img, cv2.COLOR_BGR2RGB) if len(
                resized_img.shape) == 3 else resized_img)
            pil_img.save(str(output_path), dpi=resolution,
                         format=output_format.upper())
        else:
            logger.debug(
                f"Saving image without changing resolution to format: {output_format}")
            cv2.imwrite(str(output_path), resized_img)

        logger.info(f"Image saved successfully to: {output_path}")

    input_path = Path(input_image_path)
    output_path = Path(output_image_path)

    # Batch processing mode
    if batch_mode:
        logger.info("Batch mode enabled.")
        if not input_path.is_dir():
            logger.error(
                "In batch mode, input_image_path must be a directory.")
            raise ValueError(
                "In batch mode, input_image_path must be a directory.")
        if not output_path.exists():
            logger.debug(f"Creating output directory: {output_path}")
            output_path.mkdir(parents=True, exist_ok=True)
        image_files = list(input_path.glob('*'))
        logger.debug(f"Found {len(image_files)} files to process.")
        with concurrent.futures.ThreadPoolExecutor() as executor:
            futures = []
            for file in image_files:
                if file.suffix.lower() not in ['.jpg', '.jpeg', '.png', '.tiff', '.bmp', '.gif']:
                    logger.warning(
                        f"Skipping unsupported file format: {file.name}")
                    continue
                output_file = output_path / f"{file.stem}.{output_format}"
                futures.append(executor.submit(
                    process_image, file, output_file))
            for future in concurrent.futures.as_completed(futures):
                try:
                    future.result()
                except Exception as e:
                    logger.error(f"Error processing batch image: {e}")
    else:
        logger.info("Single image mode.")
        process_image(input_path, output_path)

    logger.info("Resampling process completed successfully.")


# Example usage:
input_path = 'star_image.png'
output_path = './output/star_image_processed.png'

# Resize image, apply edge detection, adjust brightness/contrast, add watermark, sharpen, and rotate
resample_image(input_path, output_path, width=800, height=600, interpolation=cv2.INTER_CUBIC, resolution=(300, 300),
               crop_area=(100, 100, 400, 400), edge_detection=True, color_space='GRAY', add_watermark=True,
               watermark_text='Sample Watermark', watermark_position=(10, 10), watermark_opacity=0.7,
               brightness=1.2, contrast=1.5, output_format='png', sharpen=True, rotate_angle=45)
