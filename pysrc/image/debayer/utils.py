from typing import Tuple
import numpy as np

def calculate_gradients(cfa_image: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    Calculate the gradients of a CFA image.
    """
    gradient_x = np.abs(np.diff(cfa_image, axis=1))
    gradient_y = np.abs(np.diff(cfa_image, axis=0))

    gradient_x = np.pad(gradient_x, ((0, 0), (0, 1)), 'constant')
    gradient_y = np.pad(gradient_y, ((0, 1), (0, 0)), 'constant')

    return gradient_x, gradient_y

def interpolate_green_channel(cfa_image: np.ndarray, gradient_x: np.ndarray, gradient_y: np.ndarray) -> np.ndarray:
    """
    Interpolate the green channel of the CFA image.
    """
    height, width = cfa_image.shape
    green_channel = np.zeros((height, width))

    for i in range(1, height - 1):
        for j in range(1, width - 1):
            if (i % 2 == 0 and j % 2 == 1) or (i % 2 == 1 and j % 2 == 0):
                # 当前点是绿色通道点，直接赋值
                green_channel[i, j] = cfa_image[i, j]
            else:
                # 当前点不是绿色通道点，需要插值
                if gradient_x[i, j] < gradient_y[i, j]:
                    green_channel[i, j] = 0.5 * \
                        (cfa_image[i, j-1] + cfa_image[i, j+1])
                else:
                    green_channel[i, j] = 0.5 * \
                        (cfa_image[i-1, j] + cfa_image[i+1, j])

    return green_channel

def interpolate_red_blue_channel(cfa_image: np.ndarray, green_channel: np.ndarray, pattern: str) -> Tuple[np.ndarray, np.ndarray]:
    """
    Interpolate the red and blue channels of the CFA image.
    """
    height, width = cfa_image.shape
    red_channel = np.zeros((height, width))
    blue_channel = np.zeros((height, width))

    if pattern == 'BGGR':
        for i in range(0, height, 2):
            for j in range(0, width, 2):
                blue_channel[i, j] = cfa_image[i, j]
                red_channel[i+1, j+1] = cfa_image[i+1, j+1]

                green_r = 0.5 * (green_channel[i+1, j] + green_channel[i, j+1])
                green_b = 0.5 * (green_channel[i, j] + green_channel[i+1, j+1])

                blue_channel[i+1, j] = cfa_image[i+1, j] - \
                    green_b + green_channel[i+1, j]
                blue_channel[i, j+1] = cfa_image[i, j+1] - \
                    green_b + green_channel[i, j+1]
                red_channel[i, j] = cfa_image[i, j] - \
                    green_r + green_channel[i, j]
                red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                    green_r + green_channel[i+1, j+1]

    elif pattern == 'RGGB':
        for i in range(0, height, 2):
            for j in range(0, width, 2):
                red_channel[i, j] = cfa_image[i, j]
                blue_channel[i+1, j+1] = cfa_image[i+1, j+1]

                green_r = 0.5 * (green_channel[i, j+1] + green_channel[i+1, j])
                green_b = 0.5 * (green_channel[i+1, j] + green_channel[i, j+1])

                red_channel[i+1, j] = cfa_image[i+1, j] - \
                    green_r + green_channel[i+1, j]
                red_channel[i, j+1] = cfa_image[i, j+1] - \
                    green_r + green_channel[i, j+1]
                blue_channel[i, j] = cfa_image[i, j] - \
                    green_b + green_channel[i, j]
                blue_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                    green_b + green_channel[i+1, j+1]

    elif pattern == 'GBRG':
        for i in range(0, height, 2):
            for j in range(0, width, 2):
                green_channel[i, j+1] = cfa_image[i, j+1]
                blue_channel[i+1, j] = cfa_image[i+1, j]

                green_r = 0.5 * (green_channel[i, j] + green_channel[i+1, j+1])
                green_b = 0.5 * (green_channel[i+1, j] + green_channel[i, j+1])

                red_channel[i, j] = cfa_image[i, j] - \
                    green_r + green_channel[i, j]
                red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                    green_r + green_channel[i+1, j+1]
                blue_channel[i, j] = cfa_image[i, j] - \
                    green_b + green_channel[i, j]
                blue_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                    green_b + green_channel[i+1, j+1]

    elif pattern == 'GRBG':
        for i in range(0, height, 2):
            for j in range(0, width, 2):
                green_channel[i, j] = cfa_image[i, j]
                red_channel[i+1, j] = cfa_image[i+1, j]

                green_r = 0.5 * (green_channel[i, j] + green_channel[i+1, j+1])
                green_b = 0.5 * (green_channel[i+1, j] + green_channel[i, j+1])

                red_channel[i, j] = cfa_image[i, j] - \
                    green_r + green_channel[i, j]
                red_channel[i+1, j+1] = cfa_image[i+1, j+1] - \
                    green_r + green_channel[i+1, j+1]
                blue_channel[i+1, j] = cfa_image[i+1, j] - \
                    green_b + green_channel[i+1, j]
                blue_channel[i, j+1] = cfa_image[i, j+1] - \
                    green_b + green_channel[i, j+1]
