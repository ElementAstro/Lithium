import ctypes
from libs.efw import *
from loguru import logger


class ASIEFWController:
    def __init__(self):
        self.device_ids = []
        self.num_devices = 0

    def get_device_count(self):
        try:
            self.num_devices = EFWGetNum()
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
            self.device_ids = (c_int * self.num_devices)()
            result = EFWGetProductIDs(self.device_ids)
            if result != EFW_SUCCESS:
                logger.error("Failed to get device IDs")
                return False
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device IDs")
            return False

    def open_device(self, device_id):
        try:
            result = EFWOpen(device_id)
            if result != EFW_SUCCESS:
                logger.error(f"Failed to open device {device_id}")
                return False
            logger.info(f"Device {device_id} opened")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while opening device {device_id}")
            return False

    def get_device_property(self, device_id):
        try:
            device_info = EFW_INFO()
            result = EFWGetProperty(device_id, ctypes.byref(device_info))
            if result != EFW_SUCCESS:
                logger.error(f"Failed to get properties of device {device_id}")
                return None
            logger.info(f"Successfully got properties of device {device_id}")
            return device_info
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting properties of device {device_id}")
            return None

    def get_position(self, device_id):
        try:
            position = c_int()
            result = EFWGetPosition(device_id, ctypes.byref(position))
            if result != EFW_SUCCESS:
                logger.error(
                    f"Failed to get filter position of device {device_id}")
                return None
            logger.info(
                f"Current filter position of device {device_id}: {position.value}")
            return position.value
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting filter position of device {device_id}")
            return None

    def set_position(self, device_id, new_position):
        try:
            result = EFWSetPosition(device_id, new_position)
            if result != EFW_SUCCESS:
                logger.error(
                    f"Failed to set filter position of device {device_id}")
                return False
            logger.info(
                f"Filter position of device {device_id} set to: {new_position}")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while setting filter position of device {device_id}")
            return False

    def calibrate_device(self, device_id):
        try:
            result = EFWCalibrate(device_id)
            if result != EFW_SUCCESS:
                logger.error(f"Failed to calibrate device {device_id}")
                return False
            logger.info(f"Device {device_id} calibrated successfully")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while calibrating device {device_id}")
            return False

    def close_device(self, device_id):
        try:
            result = EFWClose(device_id)
            if result != EFW_SUCCESS:
                logger.error(f"Failed to close device {device_id}")
                return False
            logger.info(f"Device {device_id} closed")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while closing device {device_id}")
            return False


def main():
    controller = ASIEFWController()

    if not controller.get_device_count():
        return

    if not controller.get_device_ids():
        return

    for i in range(controller.num_devices):
        device_id = controller.device_ids[i]
        logger.info(f"Device {i} ID: {device_id}")

        if not controller.open_device(device_id):
            continue

        device_info = controller.get_device_property(device_id)
        if device_info:
            logger.info(
                f"Device {device_id} Name: {device_info.Name.decode()}")
            logger.info(
                f"Device {device_id} Slot Number: {device_info.slotNum}")

        position = controller.get_position(device_id)
        if position is not None:
            logger.info(
                f"Current filter position of device {device_id}: {position}")

        new_position = 0
        if controller.set_position(device_id, new_position):
            logger.info(
                f"Filter position of device {device_id} set to: {new_position}")

        if controller.calibrate_device(device_id):
            logger.info(f"Device {device_id} calibrated successfully")

        if controller.close_device(device_id):
            logger.info(f"Device {device_id} closed")


if __name__ == "__main__":
    main()
