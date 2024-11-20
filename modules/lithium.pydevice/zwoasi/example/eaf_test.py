import ctypes
from asi_eaf import *
from loguru import logger

class ASIEAFController:
    def __init__(self):
        self.device_ids = []
        self.num_devices = 0

    def get_device_count(self):
        try:
            self.num_devices = EAFGetNum()
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
            result = EAFGetProductIDs(self.device_ids)
            if result != EAF_SUCCESS:
                logger.error("Failed to get device IDs")
                return False
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device IDs")
            return False

    def open_device(self, device_id):
        try:
            result = EAFOpen(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to open device {device_id}")
                return False
            logger.info(f"Device {device_id} opened")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while opening device {device_id}")
            return False

    def get_device_property(self, device_id):
        try:
            device_info = EAF_INFO()
            result = EAFGetProperty(device_id, ctypes.byref(device_info))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get properties of device {device_id}")
                return None
            logger.info(f"Successfully got properties of device {device_id}")
            return device_info
        except Exception as e:
            logger.exception(f"Exception occurred while getting properties of device {device_id}")
            return None

    def move_device(self, device_id, position):
        try:
            result = EAFMove(device_id, position)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to move device {device_id} to position {position}")
                return False
            logger.info(f"Device {device_id} moved to position {position}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while moving device {device_id}")
            return False

    def stop_device(self, device_id):
        try:
            result = EAFStop(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to stop device {device_id}")
                return False
            logger.info(f"Device {device_id} stopped")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while stopping device {device_id}")
            return False

    def get_position(self, device_id):
        try:
            position = c_int()
            result = EAFGetPosition(device_id, ctypes.byref(position))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get position of device {device_id}")
                return None
            logger.info(f"Current position of device {device_id}: {position.value}")
            return position.value
        except Exception as e:
            logger.exception(f"Exception occurred while getting position of device {device_id}")
            return None

    def reset_position(self, device_id, position):
        try:
            result = EAFResetPostion(device_id, position)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to reset position of device {device_id} to {position}")
                return False
            logger.info(f"Position of device {device_id} reset to {position}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while resetting position of device {device_id}")
            return False

    def get_temperature(self, device_id):
        try:
            temperature = c_float()
            result = EAFGetTemp(device_id, ctypes.byref(temperature))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get temperature of device {device_id}")
                return None
            logger.info(f"Current temperature of device {device_id}: {temperature.value}°C")
            return temperature.value
        except Exception as e:
            logger.exception(f"Exception occurred while getting temperature of device {device_id}")
            return None

    def set_max_step(self, device_id, max_step):
        try:
            result = EAFSetMaxStep(device_id, max_step)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to set max step of device {device_id} to {max_step}")
                return False
            logger.info(f"Max step of device {device_id} set to {max_step}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while setting max step of device {device_id}")
            return False

    def get_max_step(self, device_id):
        try:
            max_step = c_int()
            result = EAFGetMaxStep(device_id, ctypes.byref(max_step))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get max step of device {device_id}")
                return None
            logger.info(f"Max step of device {device_id}: {max_step.value}")
            return max_step.value
        except Exception as e:
            logger.exception(f"Exception occurred while getting max step of device {device_id}")
            return None

    def set_backlash(self, device_id, backlash):
        try:
            result = EAFSetBacklash(device_id, backlash)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to set backlash of device {device_id} to {backlash}")
                return False
            logger.info(f"Backlash of device {device_id} set to {backlash}")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while setting backlash of device {device_id}")
            return False

    def get_backlash(self, device_id):
        try:
            backlash = c_int()
            result = EAFGetBacklash(device_id, ctypes.byref(backlash))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get backlash of device {device_id}")
                return None
            logger.info(f"Backlash of device {device_id}: {backlash.value}")
            return backlash.value
        except Exception as e:
            logger.exception(f"Exception occurred while getting backlash of device {device_id}")
            return None

    def close_device(self, device_id):
        try:
            result = EAFClose(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to close device {device_id}")
                return False
            logger.info(f"Device {device_id} closed")
            return True
        except Exception as e:
            logger.exception(f"Exception occurred while closing device {device_id}")
            return False


def main():
    controller = ASIEAFController()

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
            logger.info(f"Device {device_id} Name: {device_info.Name.decode()}")
            logger.info(f"Device {device_id} Max Step: {device_info.MaxStep}")

        position = controller.get_position(device_id)
        if position is not None:
            logger.info(f"Current position of device {device_id}: {position}")

        new_position = 1000
        if controller.move_device(device_id, new_position):
            logger.info(f"Device {device_id} moved to position: {new_position}")

        if controller.stop_device(device_id):
            logger.info(f"Device {device_id} stopped")

        temperature = controller.get_temperature(device_id)
        if temperature is not None:
            logger.info(f"Current temperature of device {device_id}: {temperature}°C")

        max_step = controller.get_max_step(device_id)
        if max_step is not None:
            logger.info(f"Max step of device {device_id}: {max_step}")

        backlash = controller.get_backlash(device_id)
        if backlash is not None:
            logger.info(f"Backlash of device {device_id}: {backlash}")

        if controller.close_device(device_id):
            logger.info(f"Device {device_id} closed")


if __name__ == "__main__":
    main()