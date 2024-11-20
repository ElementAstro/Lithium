import ctypes
from typing import List, Tuple, Dict, Optional
from loguru import logger
import argparse
from ..libs.camera import (
    ASI_CONTROL_CAPS, ASIGetNumOfConnectedCameras, ASIGetCameraProperty, ASI_CAMERA_INFO,
    ASIOpenCamera, ASICloseCamera, ASI_SUCCESS, ASIGetControlCaps, ASIGetControlValue,
    ASISetControlValue, ASIGetROIFormat, ASISetROIFormat, ASIGetStartPos, ASISetStartPos, ASIGetNumOfControls
)


def search_cameras() -> List[Dict[str, any]]:
    """
    Search for connected cameras and retrieve their properties.

    Returns:
        List[Dict[str, any]]: A list of dictionaries containing camera properties.
    """
    num_cameras = ASIGetNumOfConnectedCameras()
    logger.info(f"Number of connected cameras: {num_cameras}")

    cameras = []

    for i in range(num_cameras):
        camera_info = ASI_CAMERA_INFO()
        result = ASIGetCameraProperty(ctypes.byref(camera_info), i)
        if result == ASI_SUCCESS:
            camera_details = {
                "Name": camera_info.Name.decode(),
                "CameraID": camera_info.CameraID,
                "MaxHeight": camera_info.MaxHeight,
                "MaxWidth": camera_info.MaxWidth,
                "IsColorCam": camera_info.IsColorCam,
                "BayerPattern": camera_info.BayerPattern,
                "PixelSize": camera_info.PixelSize,
                "MechanicalShutter": camera_info.MechanicalShutter,
                "ST4Port": camera_info.ST4Port,
                "IsCoolerCam": camera_info.IsCoolerCam,
                "IsUSB3Host": camera_info.IsUSB3Host,
                "IsUSB3Camera": camera_info.IsUSB3Camera,
                "ElecPerADU": camera_info.ElecPerADU,
                "BitDepth": camera_info.BitDepth,
                "IsTriggerCam": camera_info.IsTriggerCam,
            }
            cameras.append(camera_details)
            logger.info(f"Camera {i}: {camera_details}")
        else:
            logger.error(
                f"Failed to get camera property for camera {i}. Error code: {result}")

    return cameras


def get_camera_controls(camera_id: int) -> List[Dict[str, any]]:
    """
    Get the control capabilities of a specific camera.

    Args:
        camera_id (int): The ID of the camera.

    Returns:
        List[Dict[str, any]]: A list of dictionaries containing control capabilities.
    """
    num_controls = ctypes.c_int()
    result = ASIGetNumOfControls(camera_id, ctypes.byref(num_controls))
    if result != ASI_SUCCESS:
        logger.error(
            f"Failed to get number of controls for camera {camera_id}. Error code: {result}")
        return []

    controls = []
    for i in range(num_controls.value):
        control_caps = ASI_CONTROL_CAPS()
        result = ASIGetControlCaps(camera_id, i, ctypes.byref(control_caps))
        if result == ASI_SUCCESS:
            control_details = {
                "Name": control_caps.Name.decode(),
                "Description": control_caps.Description.decode(),
                "MaxValue": control_caps.MaxValue,
                "MinValue": control_caps.MinValue,
                "DefaultValue": control_caps.DefaultValue,
                "IsAutoSupported": control_caps.IsAutoSupported,
                "IsWritable": control_caps.IsWritable,
                "ControlType": control_caps.ControlType,
            }
            controls.append(control_details)
            logger.info(f"Control {i}: {control_details}")
        else:
            logger.error(
                f"Failed to get control caps for control {i} of camera {camera_id}. Error code: {result}")

    return controls


def open_camera(camera_id: int) -> int:
    """
    Open a specific camera.

    Args:
        camera_id (int): The ID of the camera.

    Returns:
        int: The result code of the operation.
    """
    result = ASIOpenCamera(camera_id)
    if result == ASI_SUCCESS:
        logger.info(f"Camera {camera_id} opened successfully.")
    else:
        logger.error(
            f"Failed to open camera {camera_id}. Error code: {result}")
    return result


def close_camera(camera_id: int) -> int:
    """
    Close a specific camera.

    Args:
        camera_id (int): The ID of the camera.

    Returns:
        int: The result code of the operation.
    """
    result = ASICloseCamera(camera_id)
    if result == ASI_SUCCESS:
        logger.info(f"Camera {camera_id} closed successfully.")
    else:
        logger.error(
            f"Failed to close camera {camera_id}. Error code: {result}")
    return result


def get_control_value(camera_id: int, control_type: int) -> Tuple[Optional[int], Optional[int]]:
    """
    Get the value of a specific control of a camera.

    Args:
        camera_id (int): The ID of the camera.
        control_type (int): The type of the control.

    Returns:
        Tuple[Optional[int], Optional[int]]: The value and auto status of the control.
    """
    value = ctypes.c_long()
    is_auto = ctypes.c_int()
    result = ASIGetControlValue(
        camera_id, control_type, ctypes.byref(value), ctypes.byref(is_auto))
    if result == ASI_SUCCESS:
        logger.info(
            f"Control value for type {control_type} of camera {camera_id}: {value.value}, Auto: {is_auto.value}")
        return value.value, is_auto.value
    else:
        logger.error(
            f"Failed to get control value for type {control_type} of camera {camera_id}. Error code: {result}")
        return None, None


def set_control_value(camera_id: int, control_type: int, value: int, is_auto: int) -> None:
    """
    Set the value of a specific control of a camera.

    Args:
        camera_id (int): The ID of the camera.
        control_type (int): The type of the control.
        value (int): The value to set.
        is_auto (int): The auto status to set.
    """
    result = ASISetControlValue(camera_id, control_type, value, is_auto)
    if result == ASI_SUCCESS:
        logger.info(
            f"Set control value for type {control_type} of camera {camera_id} to {value}, Auto: {is_auto}")
    else:
        logger.error(
            f"Failed to set control value for type {control_type} of camera {camera_id}. Error code: {result}")


def get_roi_format(camera_id: int) -> Tuple[Optional[int], Optional[int], Optional[int], Optional[int]]:
    """
    Get the ROI format of a camera.

    Args:
        camera_id (int): The ID of the camera.

    Returns:
        Tuple[Optional[int], Optional[int], Optional[int], Optional[int]]: The width, height, binning, and image type of the ROI format.
    """
    width = ctypes.c_int()
    height = ctypes.c_int()
    binning = ctypes.c_int()
    image_type = ctypes.c_int()
    result = ASIGetROIFormat(camera_id, ctypes.byref(width), ctypes.byref(
        height), ctypes.byref(binning), ctypes.byref(image_type))
    if result == ASI_SUCCESS:
        logger.info(
            f"ROI format for camera {camera_id}: Width: {width.value}, Height: {height.value}, Binning: {binning.value}, Image Type: {image_type.value}")
        return width.value, height.value, binning.value, image_type.value
    else:
        logger.error(
            f"Failed to get ROI format for camera {camera_id}. Error code: {result}")
        return None, None, None, None


def set_roi_format(camera_id: int, width: int, height: int, binning: int, image_type: int) -> None:
    """
    Set the ROI format of a camera.

    Args:
        camera_id (int): The ID of the camera.
        width (int): The width to set.
        height (int): The height to set.
        binning (int): The binning to set.
        image_type (int): The image type to set.
    """
    result = ASISetROIFormat(camera_id, width, height, binning, image_type)
    if result == ASI_SUCCESS:
        logger.info(
            f"Set ROI format for camera {camera_id} to Width: {width}, Height: {height}, Binning: {binning}, Image Type: {image_type}")
    else:
        logger.error(
            f"Failed to set ROI format for camera {camera_id}. Error code: {result}")


def get_start_pos(camera_id: int) -> Tuple[Optional[int], Optional[int]]:
    """
    Get the start position of a camera.

    Args:
        camera_id (int): The ID of the camera.

    Returns:
        Tuple[Optional[int], Optional[int]]: The X and Y start positions.
    """
    start_x = ctypes.c_int()
    start_y = ctypes.c_int()
    result = ASIGetStartPos(camera_id, ctypes.byref(
        start_x), ctypes.byref(start_y))
    if result == ASI_SUCCESS:
        logger.info(
            f"Start position for camera {camera_id}: X: {start_x.value}, Y: {start_y.value}")
        return start_x.value, start_y.value
    else:
        logger.error(
            f"Failed to get start position for camera {camera_id}. Error code: {result}")
        return None, None


def set_start_pos(camera_id: int, start_x: int, start_y: int) -> None:
    """
    Set the start position of a camera.

    Args:
        camera_id (int): The ID of the camera.
        start_x (int): The X start position to set.
        start_y (int): The Y start position to set.
    """
    result = ASISetStartPos(camera_id, start_x, start_y)
    if result == ASI_SUCCESS:
        logger.info(
            f"Set start position for camera {camera_id} to X: {start_x}, Y: {start_y}")
    else:
        logger.error(
            f"Failed to set start position for camera {camera_id}. Error code: {result}")


def main():
    parser = argparse.ArgumentParser(description="Camera control script")
    parser.add_argument("--search", action="store_true",
                        help="Search for connected cameras")
    parser.add_argument("--open", type=int, help="Open a camera by ID")
    parser.add_argument("--close", type=int, help="Close a camera by ID")
    parser.add_argument("--get-controls", type=int,
                        help="Get controls of a camera by ID")
    parser.add_argument("--get-control-value", nargs=2, type=int,
                        help="Get control value of a camera by ID and control type")
    parser.add_argument("--set-control-value", nargs=4, type=int,
                        help="Set control value of a camera by ID, control type, value, and auto status")
    parser.add_argument("--get-roi", type=int,
                        help="Get ROI format of a camera by ID")
    parser.add_argument("--set-roi", nargs=5, type=int,
                        help="Set ROI format of a camera by ID, width, height, binning, and image type")
    parser.add_argument("--get-start-pos", type=int,
                        help="Get start position of a camera by ID")
    parser.add_argument("--set-start-pos", nargs=3, type=int,
                        help="Set start position of a camera by ID, start X, and start Y")

    args = parser.parse_args()

    if args.search:
        cameras = search_cameras()
        for camera in cameras:
            print(camera)

    if args.open is not None:
        open_camera(args.open)

    if args.close is not None:
        close_camera(args.close)

    if args.get_controls is not None:
        controls = get_camera_controls(args.get_controls)
        for control in controls:
            print(control)

    if args.get_control_value is not None:
        camera_id, control_type = args.get_control_value
        value, is_auto = get_control_value(camera_id, control_type)
        print(f"Value: {value}, Auto: {is_auto}")

    if args.set_control_value is not None:
        camera_id, control_type, value, is_auto = args.set_control_value
        set_control_value(camera_id, control_type, value, is_auto)

    if args.get_roi is not None:
        width, height, binning, image_type = get_roi_format(args.get_roi)
        print(
            f"Width: {width}, Height: {height}, Binning: {binning}, Image Type: {image_type}")

    if args.set_roi is not None:
        camera_id, width, height, binning, image_type = args.set_roi
        set_roi_format(camera_id, width, height, binning, image_type)

    if args.get_start_pos is not None:
        start_x, start_y = get_start_pos(args.get_start_pos)
        print(f"Start X: {start_x}, Start Y: {start_y}")

    if args.set_start_pos is not None:
        camera_id, start_x, start_y = args.set_start_pos
        set_start_pos(camera_id, start_x, start_y)


if __name__ == "__main__":
    main()
