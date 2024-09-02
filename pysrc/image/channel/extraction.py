import cv2
import numpy as np
import os
from matplotlib import pyplot as plt


def extract_channels(image, color_space='RGB'):
    channels = {}

    if color_space == 'RGB':
        channels['R'], channels['G'], channels['B'] = cv2.split(image)

    elif color_space == 'XYZ':
        xyz_image = cv2.cvtColor(image, cv2.COLOR_BGR2XYZ)
        channels['X'], channels['Y'], channels['Z'] = cv2.split(xyz_image)

    elif color_space == 'Lab':
        lab_image = cv2.cvtColor(image, cv2.COLOR_BGR2Lab)
        channels['L*'], channels['a*'], channels['b*'] = cv2.split(lab_image)

    elif color_space == 'LCh':
        lab_image = cv2.cvtColor(image, cv2.COLOR_BGR2Lab)
        L, a, b = cv2.split(lab_image)
        h, c = cv2.cartToPolar(a, b)
        channels['L*'] = L
        channels['c*'] = c
        channels['h*'] = h

    elif color_space == 'HSV':
        hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        channels['H'], channels['S'], channels['V'] = cv2.split(hsv_image)

    elif color_space == 'HSI':
        hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        H, S, V = cv2.split(hsv_image)
        I = V.copy()
        channels['H'] = H
        channels['Si'] = S
        channels['I'] = I

    elif color_space == 'YUV':
        yuv_image = cv2.cvtColor(image, cv2.COLOR_BGR2YUV)
        channels['Y'], channels['U'], channels['V'] = cv2.split(yuv_image)

    elif color_space == 'YCbCr':
        ycbcr_image = cv2.cvtColor(image, cv2.COLOR_BGR2YCrCb)
        channels['Y'], channels['Cb'], channels['Cr'] = cv2.split(ycbcr_image)

    elif color_space == 'HSL':
        hsl_image = cv2.cvtColor(image, cv2.COLOR_BGR2HLS)
        channels['H'], channels['S'], channels['L'] = cv2.split(hsl_image)

    elif color_space == 'CMYK':
        # Trick: Convert to CMY via XYZ
        cmyk_image = cv2.cvtColor(image, cv2.COLOR_BGR2XYZ)
        C, M, Y = cv2.split(255 - cmyk_image)
        K = np.minimum(C, np.minimum(M, Y))
        channels['C'] = C
        channels['M'] = M
        channels['Y'] = Y
        channels['K'] = K

    return channels


def show_histogram(channel_data, title='Channel Histogram'):
    plt.figure()
    plt.title(title)
    plt.xlabel('Pixel Value')
    plt.ylabel('Frequency')
    plt.hist(channel_data.ravel(), bins=256, range=[0, 256])
    plt.show()


def merge_channels(channels):
    merged_image = None
    channel_list = list(channels.values())
    if len(channel_list) >= 3:
        merged_image = cv2.merge(channel_list[:3])
    elif len(channel_list) == 2:
        merged_image = cv2.merge(
            [channel_list[0], channel_list[1], np.zeros_like(channel_list[0])])
    elif len(channel_list) == 1:
        merged_image = channel_list[0]
    return merged_image


def process_directory(input_dir, output_dir, color_space='RGB'):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for filename in os.listdir(input_dir):
        if filename.endswith(('.png', '.jpg', '.jpeg', '.bmp', '.tiff')):
            image_path = os.path.join(input_dir, filename)
            image = cv2.imread(image_path)
            base_name = os.path.splitext(filename)[0]
            extracted_channels = extract_channels(image, color_space)
            for channel_name, channel_data in extracted_channels.items():
                save_path = os.path.join(
                    output_dir, f"{base_name}_{channel_name}.png")
                cv2.imwrite(save_path, channel_data)
                show_histogram(
                    channel_data, title=f"{base_name} - {channel_name}")
                print(f"Saved {save_path}")


def save_channels(channels, base_name='output'):
    for channel_name, channel_data in channels.items():
        filename = f"{base_name}_{channel_name}.png"
        cv2.imwrite(filename, channel_data)
        print(f"Saved {filename}")


def display_image(title, image):
    cv2.imshow(title, image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


# Example usage:
image = cv2.imread('input_image.png')
extracted_channels = extract_channels(image, color_space='Lab')

# Show histograms
for name, channel in extracted_channels.items():
    show_histogram(channel, title=f"{name} Histogram")

# Save channels
save_channels(extracted_channels, base_name='output_image')

# Merge channels
merged_image = merge_channels(extracted_channels)
if merged_image is not None:
    display_image('Merged Image', merged_image)
    cv2.imwrite('merged_image.png', merged_image)

# Process directory
process_directory('input_images', 'output_images', color_space='HSV')
