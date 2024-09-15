import cv2
import numpy as np
from PIL import Image
import os
from typing import Optional, Tuple, Literal


def resample_image(input_image_path: str,
                   output_image_path: str,
                   width: Optional[int] = None,
                   height: Optional[int] = None,
                   scale: Optional[float] = None,
                   resolution: Optional[Tuple[int, int]] = None,
                   interpolation: int = cv2.INTER_LINEAR,
                   preserve_aspect_ratio: bool = True,
                   crop_area: Optional[Tuple[int, int, int, int]] = None,
                   edge_detection: bool = False,
                   color_space: Literal['BGR', 'GRAY', 'HSV', 'RGB'] = 'BGR',
                   add_watermark: bool = False,
                   watermark_text: str = '',
                   watermark_position: Tuple[int, int] = (0, 0),
                   watermark_opacity: float = 0.5,
                   batch_mode: bool = False,
                   output_format: str = 'jpg',
                   brightness: float = 1.0,
                   contrast: float = 1.0,
                   sharpen: bool = False,
                   rotate_angle: Optional[float] = None):
    """
    Resamples an image with given dimensions, scale, resolution, and additional processing options.

    :param input_image_path: Path to the input image (or directory in batch mode)
    :param output_image_path: Path to save the resampled image(s)
    :param width: Desired width in pixels
    :param height: Desired height in pixels
    :param scale: Scale factor for resizing (e.g., 0.5 for half size, 2.0 for double size)
    :param resolution: Tuple of horizontal and vertical resolution (in dpi)
    :param interpolation: Interpolation method (e.g., cv2.INTER_LINEAR, cv2.INTER_CUBIC, cv2.INTER_NEAREST)
    :param preserve_aspect_ratio: Whether to preserve the aspect ratio of the original image
    :param crop_area: Tuple defining the crop area (x, y, w, h)
    :param edge_detection: Whether to apply edge detection before resizing
    :param color_space: Color space to convert the image to (e.g., 'GRAY', 'HSV', 'RGB')
    :param add_watermark: Whether to add a watermark to the image
    :param watermark_text: The text to be used as a watermark
    :param watermark_position: Position tuple (x, y) for the watermark
    :param watermark_opacity: Opacity level for the watermark (0.0 to 1.0)
    :param batch_mode: Whether to process multiple images in a directory
    :param output_format: Output image format (e.g., 'jpg', 'png', 'tiff')
    :param brightness: Brightness adjustment factor (1.0 = no change)
    :param contrast: Contrast adjustment factor (1.0 = no change)
    :param sharpen: Whether to apply sharpening to the image
    :param rotate_angle: Angle to rotate the image (in degrees)
    """

    def adjust_brightness_contrast(image: np.ndarray, brightness: float = 1.0, contrast: float = 1.0) -> np.ndarray:
        # Clip the pixel values to be in the valid range after adjustment
        adjusted = cv2.convertScaleAbs(
            image, alpha=contrast, beta=(brightness - 1.0) * 255)
        return adjusted

    def add_text_watermark(image: np.ndarray, text: str, position: Tuple[int, int], opacity: float) -> np.ndarray:
        overlay = image.copy()
        output = image.copy()
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 1
        thickness = 2
        text_size = cv2.getTextSize(text, font, font_scale, thickness)[0]
        text_x = position[0]
        text_y = position[1] + text_size[1]
        cv2.putText(overlay, text, (text_x, text_y), font,
                    font_scale, (255, 255, 255), thickness)
        # Combine original image with overlay
        cv2.addWeighted(overlay, opacity, output, 1 - opacity, 0, output)
        return output

    def sharpen_image(image: np.ndarray) -> np.ndarray:
        kernel = np.array([[0, -1, 0], [-1, 5, -1], [0, -1, 0]])
        sharpened = cv2.filter2D(image, -1, kernel)
        return sharpened

    def process_image(image_path: str, output_path: str):
        img = cv2.imread(image_path)
        if img is None:
            raise ValueError(f"Cannot load image from {image_path}")

        original_height, original_width = img.shape[:2]

        # Crop if needed
        if crop_area:
            x, y, w, h = crop_area
            img = img[y:y+h, x:x+w]

        # Edge detection
        if edge_detection:
            img = cv2.Canny(img, 100, 200)

        # Convert color space if needed
        # Only convert if image is not already grayscale
        if color_space == 'GRAY' and len(img.shape) == 3:
            img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        # Only convert if image has 3 channels
        elif color_space == 'HSV' and len(img.shape) == 3:
            img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # Convert to RGB if required
        elif color_space == 'RGB' and len(img.shape) == 3:
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        # Calculate new dimensions
        if scale:
            new_width = int(original_width * scale)
            new_height = int(original_height * scale)
        else:
            if width and height:
                new_width = width
                new_height = height
            elif width:
                new_width = width
                new_height = int(
                    (width / original_width) * original_height) if preserve_aspect_ratio else height
            elif height:
                new_height = height
                new_width = int((height / original_height) *
                                original_width) if preserve_aspect_ratio else width
            else:
                new_width, new_height = original_width, original_height

        # Perform resizing
        resized_img = cv2.resize(
            img, (new_width, new_height), interpolation=interpolation)

        # Adjust brightness and contrast
        resized_img = adjust_brightness_contrast(
            resized_img, brightness, contrast)

        # Apply sharpening if needed
        if sharpen:
            resized_img = sharpen_image(resized_img)

        # Rotate the image if needed
        if rotate_angle:
            center = (new_width // 2, new_height // 2)
            rotation_matrix = cv2.getRotationMatrix2D(
                center, rotate_angle, 1.0)
            resized_img = cv2.warpAffine(
                resized_img, rotation_matrix, (new_width, new_height))

        # Add watermark if needed
        if add_watermark:
            resized_img = add_text_watermark(
                resized_img, watermark_text, watermark_position, watermark_opacity)

        # Save the image
        if resolution:
            dpi_x, dpi_y = resolution
            pil_img = Image.fromarray(cv2.cvtColor(resized_img, cv2.COLOR_BGR2RGB) if len(
                resized_img.shape) == 3 else resized_img)
            pil_img.save(output_path, dpi=(dpi_x, dpi_y), format=output_format)
        else:
            cv2.imwrite(output_path, resized_img)

    # Batch processing mode
    if batch_mode:
        if not os.path.isdir(input_image_path):
            raise ValueError(
                "In batch mode, input_image_path must be a directory")
        if not os.path.exists(output_image_path):
            os.makedirs(output_image_path)
        for filename in os.listdir(input_image_path):
            file_path = os.path.join(input_image_path, filename)
            output_file_path = os.path.join(
                output_image_path, f"{os.path.splitext(filename)[0]}.{output_format}")
            process_image(file_path, output_file_path)
    else:
        process_image(input_image_path, output_image_path)


# Example usage:
input_path = 'star_image.png'
output_path = './output/star_image_processed.png'

# Resize image, apply edge detection, adjust brightness/contrast, add watermark, sharpen, and rotate
resample_image(input_path, output_path, width=800, height=600, interpolation=cv2.INTER_CUBIC, resolution=(300, 300),
               crop_area=(100, 100, 400, 400), edge_detection=True, color_space='GRAY', add_watermark=True,
               watermark_text='Sample Watermark', watermark_position=(10, 10), watermark_opacity=0.7,
               brightness=1.2, contrast=1.5, output_format='png', sharpen=True, rotate_angle=45)
