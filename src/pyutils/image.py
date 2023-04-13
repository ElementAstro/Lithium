import os
import numpy as np
from astropy.io import fits
import cv2
import math


def calcHfd(inImage, inOuterDiameter) -> float:
    Img = inImage - np.mean(inImage)
    Img[Img < 0] = 0
    width, height = Img.shape
    outerRadius = inOuterDiameter / 2
    sumValue = np.sum(Img[(((np.arange(width) - width // 2) ** 2)[:, None] + (
        (np.arange(height) - height // 2) ** 2)[None, :]) <= outerRadius ** 2])
    if sumValue > 0:
        distMat = np.sqrt((np.arange(width)[
                          :, None] - width // 2) ** 2 + (np.arange(height)[None, :] - height // 2) ** 2)
        sumDist = np.sum(Img[distMat <= outerRadius] *
                         distMat[distMat <= outerRadius])
        hfd = 2.0 * sumDist / sumValue
    else:
        hfd = math.sqrt(2.0) * outerRadius
    return hfd


def DebayerStarCountHfr(filename: str) -> tuple:
    if not filename:
        return 0, 0

    if not os.path.exists(filename):
        return 0, 0

    with fits.open(filename) as hdulist:
        img = hdulist[0].data
        img = cv2.cvtColor(img, cv2.COLOR_BayerBG2BGR)
        cv2.imwrite("tmp.png", img)
    img = cv2.imread("tmp.png")

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    _, thresh = cv2.threshold(gray, 50, 255, cv2.THRESH_BINARY)
    contours, hierarchy = cv2.findContours(
        thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    StarCount = len(contours)
    HfrList = []
    for contour in contours:
        area = cv2.contourArea(contour)
        if area >= 10:
            (x, y), radius = cv2.minEnclosingCircle(contour)
            center = (int(x), int(y))
            radius = int(radius)
            x1, y1, x2, y2 = max(0, int(x-radius-10)), max(0, int(y-radius-10)), min(
                gray.shape[1], int(x+radius+10)), min(gray.shape[0], int(y+radius+10))
            hfr = calcHfd(gray[y1:y2, x1:x2], 60) / 2
            HfrList.append(hfr)

    if not HfrList:
        return 0, 0

    AvgHfr = np.average(HfrList)
    return StarCount, AvgHfr


def StarCountHfr(filename: str):
    with fits.open(filename) as hdulist:
        img = hdulist[0].data
        img = cv2.cvtColor(img, cv2.COLOR_BayerBG2BGR)
        cv2.imwrite("tmp.png", img)
    img = cv2.imread("tmp.png")

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    _, thresh = cv2.threshold(blur, 50, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(
        thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    StarCount = len(contours)
    HfrList = []
    for contour in contours:
        area = cv2.contourArea(contour)
        if area >= 10:
            (x, y), radius = cv2.minEnclosingCircle(contour)
            hfr_radius = int(radius + 10)
            hfr = calcHfd(gray[y-hfr_radius:y+hfr_radius,
                          x-hfr_radius:x+hfr_radius], 60) / 2
            HfrList.append(hfr)

    if not HfrList:
        return 0, 0

    AvgHfr = np.average(HfrList)
    return StarCount, AvgHfr
