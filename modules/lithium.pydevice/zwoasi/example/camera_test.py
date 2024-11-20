import ctypes
from asiccd import *
from loguru import logger

class ASICameraController:
    def __init__(self):
        self.device_ids = []
        self.num_devices = 0

    def get_device_count(self):
        try:
            self.num_devices = ASIGetNumOfConnectedCameras()
            if self.num_devices <= 0:
                logger.error("No devices found")
                return False
            logger.info(f"Found {self.num_devices} devices")
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device count")
            return False

    def get_device_ids(self):
        if self.num_devices <= 0:
            return False
        try:
            self.device_ids = [ASIGetCameraProperty(i) for i in range(self.num_devices)]
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device IDs")
            return False

    def open_device(self, device_id):
        try:
            result = ASIOpenCamera(device_id)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to open device {device_id}")
                return False
            logger.info(f"Device {device_id} opened")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while opening device {device_id}")
            return False

    def get_device_property(self, device_id):
        try:
            camera_info = ASI_CAMERA_INFO()
            result = ASIGetCameraProperty(ctypes.byref(camera_info), device_id)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to get properties of device {device_id}")
                return None
            logger.info(f"Successfully got properties of device {device_id}")
            return camera_info
        except Exception as e:
            logger.exception(f"Exception occurred while getting properties of device {device_id}")
            return None

    def start_exposure(self, device_id):
        try:
            result = ASIStartExposure(device_id, ASI_FALSE)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to start exposure on device {device_id}")
                return False
            logger.info(f"Exposure started on device {device_id}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while starting exposure on device {device_id}")
            return False

    def stop_exposure(self, device_id):
        try:
            result = ASIStopExposure(device_id)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to stop exposure on device {device_id}")
                return False
            logger.info(f"Exposure stopped on device {device_id}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while stopping exposure on device {device_id}")
            return False

    def get_exposure_status(self, device_id):
        try:
            status = ASI_EXPOSURE_STATUS()
            result = ASIGetExpStatus(device_id, ctypes.byref(status))
            if result != ASI_SUCCESS:
                logger.error(f"Failed to get exposure status of device {device_id}")
                return None
            logger.info(f"Exposure status of device {device_id}: {status.value}")
            return status.value
        except Exception as e:
            logger.exception(f"Exception occurred while getting exposure status of device {device_id}")
            return None

    def get_image_data(self, device_id, width, height, bit_depth):
        try:
            buffer_size = width * height * (bit_depth // 8)
            buffer = (ctypes.c_ubyte * buffer_size)()
            result = ASIGetDataAfterExp(device_id, buffer, buffer_size)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to get image data from device {device_id}")
                return None
            logger.info(f"Successfully got image data from device {device_id}")
            return buffer
        except Exception as e:
            logger.exception(f"Exception occurred while getting image data from device {device_id}")
            return None

    def close_device(self, device_id):
        try:
            result = ASICloseCamera(device_id)
            if result != ASI_SUCCESS:
                logger.error(f"Failed to close device {device_id}")
                return False
            logger.info(f"Device {device_id} closed")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while closing device {device_id}")
            return False


def main():
    controller = ASICameraController()

    if not controller.get_device_count():
        return

    if not controller.get_device_ids():
        return

    for i in range(controller.num_devices):
        device_id = controller.device_ids[i].CameraID
        logger.info(f"Device {i} ID: {device_id}")

        if not controller.open_device(device_id):
            continue

        device_info = controller.get_device_property(device_id)
        if device_info:
            logger.info(f"Device {device_id} Name: {device_info.Name.decode()}")
            logger.info(f"Device {device_id} Max Height: {device_info.MaxHeight}")
            logger.info(f"Device {device_id} Max Width: {device_info.MaxWidth}")

        if controller.start_exposure(device_id):
            logger.info(f"Exposure started on device {device_id}")

        status = controller.get_exposure_status(device_id)
        if status is not None:
            logger.info(f"Exposure status of device {device_id}: {status}")

        if controller.stop_exposure(device_id):
            logger.info(f"Exposure stopped on device {device_id}")

        width, height, bit_depth = device_info.MaxWidth, device_info.MaxHeight, 16
        image_data = controller.get_image_data(device_id, width, height, bit_depth)
        if image_data is not None:
            logger.info(f"Successfully got image data from device {device_id}")

        if controller.close_device(device_id):
            logger.info(f"Device {device_id} closed")


if __name__ == "__main__":
    main()