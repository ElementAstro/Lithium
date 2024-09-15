import cv2
import numpy as np
import matplotlib.pyplot as plt

# 1. 直方图计算


def calculate_histogram(image, channel=0):
    histogram = cv2.calcHist([image], [channel], None, [256], [0, 256])
    return histogram

# 2. 显示直方图


def display_histogram(histogram, title="Histogram"):
    plt.plot(histogram)
    plt.title(title)
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Frequency')
    plt.show()

# 3. 直方图变换功能


def apply_histogram_transformation(image, shadows_clip=0.0, highlights_clip=1.0, midtones_balance=0.5, lower_bound=-1.0, upper_bound=2.0):
    # 归一化
    normalized_image = image.astype(np.float32) / 255.0

    # 阴影和高光裁剪
    clipped_image = np.clip(
        (normalized_image - shadows_clip) / (highlights_clip - shadows_clip), 0, 1)

    # 中间调平衡
    def mtf(x): return (x**midtones_balance) / \
        ((x**midtones_balance + (1-x)**midtones_balance)**(1/midtones_balance))
    transformed_image = mtf(clipped_image)

    # 动态范围扩展
    expanded_image = np.clip(
        (transformed_image - lower_bound) / (upper_bound - lower_bound), 0, 1)

    # 重新缩放至[0, 255]
    output_image = (expanded_image * 255).astype(np.uint8)
    return output_image

# 4. 自动裁剪功能


def auto_clip(image, clip_percent=0.01):
    # 计算累积分布函数 (CDF)
    hist, bins = np.histogram(image.flatten(), 256, [0, 256])
    cdf = hist.cumsum()

    # 计算裁剪点
    total_pixels = image.size
    lower_clip = np.searchsorted(cdf, total_pixels * clip_percent)
    upper_clip = np.searchsorted(cdf, total_pixels * (1 - clip_percent))

    # 应用裁剪
    auto_clipped_image = apply_histogram_transformation(
        image, shadows_clip=lower_clip/255.0, highlights_clip=upper_clip/255.0)

    return auto_clipped_image

# 5. 显示原始RGB直方图


def display_rgb_histogram(image):
    color = ('b', 'g', 'r')
    for i, col in enumerate(color):
        hist = calculate_histogram(image, channel=i)
        plt.plot(hist, color=col)
    plt.title('RGB Histogram')
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Frequency')
    plt.show()

# 6. 实时预览功能（简单模拟）


def real_time_preview(image, transformation_function, **kwargs):
    preview_image = transformation_function(image, **kwargs)
    cv2.imshow('Real-Time Preview', preview_image)


# 主程序入口
if __name__ == "__main__":
    # 加载图像
    image = cv2.imread('image.jpg')

    # 转换为灰度图像
    grayscale_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # 显示原始图像和直方图
    cv2.imshow('Original Image', image)
    histogram = calculate_histogram(grayscale_image)
    display_histogram(histogram, title="Original Grayscale Histogram")

    # 显示RGB直方图
    display_rgb_histogram(image)

    # 应用直方图变换
    transformed_image = apply_histogram_transformation(
        grayscale_image, shadows_clip=0.1, highlights_clip=0.9, midtones_balance=0.4, lower_bound=-0.5, upper_bound=1.5)
    cv2.imshow('Transformed Image', transformed_image)

    # 显示变换后的直方图
    transformed_histogram = calculate_histogram(transformed_image)
    display_histogram(transformed_histogram,
                      title="Transformed Grayscale Histogram")

    # 应用自动裁剪
    auto_clipped_image = auto_clip(grayscale_image, clip_percent=0.01)
    cv2.imshow('Auto Clipped Image', auto_clipped_image)

    # 实时预览模拟
    real_time_preview(grayscale_image, apply_histogram_transformation,
                      shadows_clip=0.05, highlights_clip=0.95, midtones_balance=0.5)

    cv2.waitKey(0)
    cv2.destroyAllWindows()
